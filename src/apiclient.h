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

#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>

static const std::string PHONE_OS = "Android";

class ApiClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool needsAuthorization READ getNeedsAuthorization NOTIFY needsAuthorizationChanged)
public:

    explicit ApiClient(QObject *parent = nullptr);

    bool getNeedsAuthorization();

    Q_INVOKABLE void sendNumber(QString number);
    Q_INVOKABLE void sendCode(QString code);

signals:
    void error(QString message);
    void waitingForPhoneNumber();
    void waitingForCode();
    void authorized();
    void needsAuthorizationChanged();

private:
    QString _phoneNumber;
    QString _authToken;
    QString _refreshToken;
};

#endif // APICLIENT_H
