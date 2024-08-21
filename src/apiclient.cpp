/*

This file is part of Outpost.
Copyright 2023, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

Outpost is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Outpost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Yottagram. If not, see <http://www.gnu.org/licenses/>.

*/

#include <QDebug>
#include <QSettings>
#include <QString>
#include <cpr/cpr.h>
#include "apiclient.h"
#include "endpoints.h"

ApiClient::ApiClient(QObject *parent) : QObject(parent)
{
    QSettings settings;

    _phoneNumber = settings.value("phoneNumber", "").toString();
    _authToken = settings.value("authToken", "").toString();
    _refreshToken = settings.value("refreshToken", "").toString();
}

bool ApiClient::getNeedsAuthorization()
{
    return _authToken == "" || _refreshToken == "";
}

void ApiClient::sendNumber(QString number)
{
    nlohmann::json payload;

    _phoneNumber = number;
    payload["phoneNumber"]["prefix"] = "+48";
    payload["phoneNumber"]["value"] = _phoneNumber.toStdString();

    nlohmann::json response = request(Endpoints::SMS_SEND_CODE, payload.dump(), POST, false);

    if (response.is_discarded()) {
        emit waitingForCode();
    } else {
        emit error(tr("Error sending phone number"));
    }
}

void ApiClient::sendCode(QString code)
{
    nlohmann::json payload;
    payload["smsCode"] = code.toStdString();
    payload["devicePlatform"] = PHONE_OS;
    payload["phoneNumber"]["prefix"] = "+48";
    payload["phoneNumber"]["value"] = _phoneNumber.toStdString();

    nlohmann::json response = request(Endpoints::SMS_CONFIRM_CODE, payload.dump(), POST, false);

    if (!response.empty() && !response.is_discarded()) {
        _authToken = QString::fromStdString(response["authToken"]);
        _refreshToken = QString::fromStdString(response["refreshToken"]);

        QSettings settings;
        settings.setValue("phoneNumber", _phoneNumber);
        settings.setValue("authToken", _authToken);
        settings.setValue("refreshToken", _refreshToken);

        emit needsAuthorizationChanged();
        emit authorized();
    } else {
        emit error(tr("Error sending code"));
    }
}

void ApiClient::logout()
{
    nlohmann::json response = request(Endpoints::LOGOUT, "", POST, true);

    _authToken = "";
    _refreshToken = "";
    _phoneNumber = "";

    QSettings settings;
    settings.setValue("phoneNumber", _phoneNumber);
    settings.setValue("authToken", _authToken);
    settings.setValue("refreshToken", _refreshToken);

    emit needsAuthorizationChanged();
}

void ApiClient::track(QString number)
{
    nlohmann::json payload;
    payload["shipmentNumber"] = number.toStdString();

    nlohmann::json response = request(Endpoints::OBSERVED_PARCEL, payload.dump(), POST, true);

    if (!response.empty() && !response.is_discarded()) {
        emit refresh();
    } else {
        emit error(tr("Error sending code"));
    }
}

void ApiClient::stopTracking(QString number)
{
    nlohmann::json response = request(Endpoints::OBSERVED_PARCEL + "/" + number.toStdString(), "", DELETE, true);

    emit refresh();
}

nlohmann::json ApiClient::getParcels(ParcelListType parcelType)
{
    std::string url;

    switch (parcelType) {
    case Pending:
    case Tracked:
        url = Endpoints::PARCELS;
        break;
    case Sent:
        url = Endpoints::SENT;
        break;
    case Returns:
        url = Endpoints::RETURNS;
        break;
    }

    return request(url, "", GET, true);
}

nlohmann::json ApiClient::request(std::string url, std::string body, RequestType type, bool withAuth)
{
    cpr::Response r;

    auto doRequest = [this](std::string url, std::string body, RequestType type, bool withAuth) {
        cpr::Response r;
        cpr::Header header;
        header["Content-Type"] = "application/json; charset=UTF-8";
        header["User-Agent"] = "InPost-Mobile/3.23.0(32300001) (Android 9; unknown; unknown unknown; en)";
        if (withAuth)
            header["Authorization"] = _authToken.toStdString();

        switch (type) {
        case GET:
            r = cpr::Get(cpr::Url{url},
                         header);
            break;
        case POST:
            r = cpr::Post(cpr::Url{url},
                          cpr::Body{body},
                          header);
            break;
        case DELETE:
            r = cpr::Delete(cpr::Url{url},
                          header);
        }

        return r;
    };

    r = doRequest(url, body, type, withAuth);
    qDebug() << "Status code: " << r.status_code;

    if (r.status_code == 401) {
        bool ret = refreshToken();
        if (!ret) return {};

        r = doRequest(url, body, type, withAuth);
    }

    switch (r.status_code) {
    case 200:
        qDebug() << QString::fromStdString(r.text);
        return nlohmann::json::parse(r.text, nullptr, false);
    case 401:
        _refreshToken = "";
        _authToken = "";
        emit needsAuthorizationChanged();
    case 429:
        emit error(tr("Error too many requests"));
    }

    return {};
}

bool ApiClient::refreshToken()
{
    nlohmann::json payload;
    payload["refreshToken"] = _refreshToken.toStdString();
    payload["phoneOS"] = PHONE_OS;

    cpr::Response r = cpr::Post(cpr::Url{Endpoints::REFRESH_TOKEN},
                  cpr::Body{payload.dump()},
                  cpr::Header{{"Content-Type", "application/json"}} );

    if (r.status_code == 200) {
        nlohmann::json data = nlohmann::json::parse(r.text);

        if (data["reauthenticationRequired"]) {
            _refreshToken = "";
            _authToken = "";
            emit needsAuthorizationChanged();

            return false;
        }

        _authToken = QString::fromStdString(data["authToken"]);
        return true;
    }

    return false;
}
