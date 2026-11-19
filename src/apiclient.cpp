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

#include <QCryptographicHash>
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <cpr/cpr.h>
#include <random>
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

QByteArray ApiClient::randomUrlSafeString(int bytes)
{
    std::random_device random;
    QByteArray data;

    for (int i = 0; i < bytes; i++) {
        data.append(static_cast<char>(random() & 0xFF));
    }

    return data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

QString ApiClient::authorizeUrl()
{
    _codeVerifier = randomUrlSafeString(32);
    _state = randomUrlSafeString(16);
    QByteArray codeChallenge = QCryptographicHash::hash(_codeVerifier, QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QUrlQuery query;
    query.addQueryItem("client_id", "inpost-mobile");
    query.addQueryItem("response_type", "code");
    query.addQueryItem("redirect_uri", QString::fromStdString(Endpoints::AUTHORIZE_REDIRECT));
    query.addQueryItem("scope", "openid");
    query.addQueryItem("code_challenge", codeChallenge);
    query.addQueryItem("code_challenge_method", "S256");
    query.addQueryItem("nonce", randomUrlSafeString(16));
    query.addQueryItem("state", _state);

    QUrl url(QString::fromStdString(Endpoints::AUTHORIZE_URL));
    url.setQuery(query);

    qDebug() << url.toString(QUrl::FullyEncoded);

    return url.toString(QUrl::FullyEncoded);
}

void ApiClient::handleCallback(QString callbackUrl)
{
    if (_codeVerifier.isEmpty()) return;

    QUrlQuery query(QUrl(callbackUrl).query());
    QString code = query.queryItemValue("code");
    QString state = query.queryItemValue("state");

    if (code.isEmpty() || state.toUtf8() != _state) {
        emit error(tr("Login failed"));
        return;
    }

    cpr::Response r = cpr::Post(cpr::Url{Endpoints::OAUTH_TOKEN},
                                cpr::Payload{{"client_id", "inpost-mobile"},
                                             {"grant_type", "authorization_code"},
                                             {"redirect_uri", Endpoints::AUTHORIZE_REDIRECT},
                                             {"code_verifier", _codeVerifier.toStdString()},
                                             {"code", code.toStdString()}},
                                cpr::Header{{"User-Agent", "InPost-Mobile/4.23.0(42300002) (Android 9; unknown; unknown unknown; en)"}});

    _codeVerifier.clear();
    _state.clear();

    qDebug() << "Token status code: " << r.status_code << QString::fromStdString(r.text);

    nlohmann::json data = nlohmann::json::parse(r.text, nullptr, false);
    if (r.status_code != 200 || data.is_discarded() || !data.contains("access_token") || !data.contains("refresh_token")) {
        emit error(tr("Login failed"));
        return;
    }

    _authToken = QString::fromStdString(data["access_token"].get<std::string>());
    _refreshToken = QString::fromStdString(data["refresh_token"].get<std::string>());

    QSettings settings;
    settings.setValue("authToken", _authToken);
    settings.setValue("refreshToken", _refreshToken);

    emit needsAuthorizationChanged();
    emit authorized();
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
        header["User-Agent"] = "InPost-Mobile/4.23.0(42300002) (Android 9; unknown; unknown unknown; en)";
        if (withAuth)
            header["Authorization"] = "Bearer " + _authToken.toStdString();

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
    cpr::Response r = cpr::Post(cpr::Url{Endpoints::OAUTH_TOKEN},
                                cpr::Payload{{"client_id", "inpost-mobile"},
                                             {"grant_type", "refresh_token"},
                                             {"refresh_token", _refreshToken.toStdString()}},
                                cpr::Header{{"User-Agent", "InPost-Mobile/4.23.0(42300002) (Android 9; unknown; unknown unknown; en)"}});

    qDebug() << "Refresh status code: " << r.status_code;

    nlohmann::json data = nlohmann::json::parse(r.text, nullptr, false);

    if (r.status_code == 200 && !data.is_discarded() && data.contains("access_token")) {
        _authToken = QString::fromStdString(data["access_token"].get<std::string>());
        if (data.contains("refresh_token"))
            _refreshToken = QString::fromStdString(data["refresh_token"].get<std::string>());

        QSettings settings;
        settings.setValue("authToken", _authToken);
        settings.setValue("refreshToken", _refreshToken);

        return true;
    }

    if (r.status_code == 400 || r.status_code == 401) {
        _refreshToken = "";
        _authToken = "";

        QSettings settings;
        settings.setValue("authToken", _authToken);
        settings.setValue("refreshToken", _refreshToken);

        emit needsAuthorizationChanged();
    }

    return false;
}
