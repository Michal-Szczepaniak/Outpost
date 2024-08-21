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

#include <QtQuick>
#include <sailfishapp.h>
#include "apiclient.h"
#include "parcellist.h"
#include "QZXing.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QSharedPointer<QQuickView> view(SailfishApp::createView());

    QZXing::registerQMLTypes();
    QZXing::registerQMLImageProvider(*view->engine());

    ApiClient client;
    ParcelList parcelList(&client);

    view->rootContext()->setContextProperty("api", &client);
    view->rootContext()->setContextProperty("parcelList", &parcelList);

    qmlRegisterType<ParcelList>("com.verdanditeam.outpost", 1, 0, "ParcelList");


    view->setSource(SailfishApp::pathTo("qml/outpost.qml"));
    view->show();

    return app->exec();
}
