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

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    Connections {
        target: api

        onError: Notices.show(message, Notice.Short, Notice.Center)

        onAuthorized: {
            pageStack.replace(Qt.resolvedUrl("Main.qml"))
        }

        onWaitingForPhoneNumber: {
            pageStack.replace(Qt.resolvedUrl("PhoneNumberDialog.qml"))
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Login")
            }

            Label {
                x: Theme.horizontalPageMargin
                text: qsTr("Code")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }

            TextField {
                id: code
                width: column.width
                inputMethodHints: Qt.ImhDigitsOnly
                Keys.onReturnPressed: {
                    api.sendCode(code.text)
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Submit")
                onClicked: api.sendCode(code.text)
            }
        }
    }
}
