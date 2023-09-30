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

#include "apiclient.h"
#include "endpoints.h"
#include <cpr/cpr.h>
#include <QDebug>
#include <QSettings>
#include <nlohmann/json.hpp>

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
    payload["phoneNumber"] = _phoneNumber.toStdString();


    cpr::Response r = cpr::Post(cpr::Url{Endpoints::SMS_SEND_CODE},
                                cpr::Body{payload.dump()},
                                cpr::Header{{"Content-Type", "application/json"}} );

    if (r.status_code == 200) {
        emit waitingForCode();
    } else {
        emit error(tr("Error sending phone number"));
    }
}

void ApiClient::sendCode(QString code)
{
    nlohmann::json payload;
    payload["phoneNumber"] = _phoneNumber.toStdString();
    payload["smsCode"] = code.toStdString();
    payload["phoneOS"] = PHONE_OS;

    cpr::Response r = cpr::Post(cpr::Url{Endpoints::SMS_CONFIRM_CODE},
                                cpr::Body{payload.dump()},
                                cpr::Header{{"Content-Type", "application/json"}} );

    if (r.status_code == 200) {
        nlohmann::json data = nlohmann::json::parse(r.text);
        _authToken = QString::fromStdString(data["authToken"]);
        _refreshToken = QString::fromStdString(data["refreshToken"]);

        QSettings settings;
        settings.setValue("phoneNumber", _phoneNumber);
        settings.setValue("authToken", _authToken);
        settings.setValue("refreshToken", _refreshToken);

        emit needsAuthorizationChanged();
        emit authorized();
    } else {
        switch (r.status_code) {
        case 429:
            emit error(tr("Error too many requests"));
            break;
        default:
            emit error(tr("Error sending code"));
            break;
        }
    }
}
