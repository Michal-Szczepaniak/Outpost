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
import com.verdanditeam.outpost 1.0

Page {
    id: page

    allowedOrientations: Orientation.Portrait
    property bool initialized: false

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (api.needsAuthorization) {
                pageStack.replace(Qt.resolvedUrl("PhoneNumberDialog.qml"))
            } else {
                loadTimer.start();
            }
        }
    }

    Connections {
        target: api

        onRefresh: parcelList.load(listTypeSelect.currentIndex)
        onError: Notices.show(message, Notice.Short, Notice.Center)
        onNeedsAuthorizationChanged: {
            pageStack.replace(Qt.resolvedUrl("PhoneNumberDialog.qml"))
        }
    }

    Timer {
        id: loadTimer
        interval: 100
        repeat: false
        onTriggered: {
            if (!initialized) {
                parcelList.load();
                initialized = true
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PageHeader {
            id: header
            title: qsTr("Parcels")
        }

        Component {
            id: trackDialog
            Dialog {
                allowedOrientations: Orientation.Portrait
                onAccepted: api.track(trackNumberField.text)

                Column {
                    anchors {
                        bottom: parent.bottom
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        topMargin: Theme.paddingLarge*4
                    }
                    spacing: Theme.paddingLarge

                    Label {
                        x: Theme.horizontalPageMargin
                        text: qsTr("Package number to track:")
                        font.family: Theme.fontFamilyHeading
                        color: Theme.highlightColor
                    }

                    TextField {
                        id: trackNumberField
                        width: parent.width - Theme.horizontalPageMargin*2
                        inputMethodHints: Qt.ImhDialableCharactersOnly
                        validator: RegExpValidator { regExp: /^[\+0-9]{24}$/ }
                        description: qsTr("Package number")
                    }
                }
            }
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("About.qml"))
            }

            MenuItem {
                text: qsTr("Logout")
                visible: !api.needsAuthorization
                onClicked: api.logout()
            }

            MenuItem {
                text: qsTr("Track package number")
                visible: !api.needsAuthorization
                onClicked: pageStack.push(trackDialog)
            }

            MenuItem {
                text: qsTr("Reload")
                visible: !api.needsAuthorization
                onClicked: parcelList.load(listTypeSelect.currentIndex)
            }
        }

        ComboBox {
            id: listTypeSelect
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: Theme.paddingMedium

            label: qsTr("List type")

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Pending")
                }
                MenuItem {
                    text: qsTr("Track")
                }
                MenuItem {
                    text: qsTr("Sent")
                }
                MenuItem {
                    text: qsTr("Returns")
                }
            }

            onCurrentIndexChanged: {
                parcelList.load(currentIndex)
            }
        }

        SilicaListView {
            id: parcelListView
            width: parent.width
            model: parcelList
            anchors.top: parent.top
            anchors.topMargin: Math.max(header.height, listTypeSelect.height)
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            clip: true

            ViewPlaceholder {
                enabled: parcelListView.count === 0
                text: "No content"
                hintText: "Try selecting different category"
            }

            delegate: ListItem {
                id: listItem
                contentHeight: labelColumn.height + sectionHeader.height + Theme.paddingMedium*2

                menu: ContextMenu {
                    hasContent: qrCode !== "" || openCode !== "" || parcelOwnership === 2

                    MenuItem {
                        text: qsTr("Show QR code")
                        visible: qrCode !== "" || openCode !== ""
                        onClicked: pageStack.push(qrDialog)
                    }

                    MenuItem {
                        text: qsTr("Stop tracking")
                        visible: parcelOwnership === 2
                        onClicked: api.stopTracking(shipmentNumber)
                    }
                }

                Component {
                    id: qrDialog
                    Dialog {
                        allowedOrientations: Orientation.Portrait

                        Image {
                            id: image
                            anchors.centerIn: parent
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: Math.min(parent.width, parent.height)
                            sourceSize.height: Math.min(parent.width, parent.height)
                            source: "image://QZXing/encode/" + qrCode + "?correctionLevel=H&border=true"
                            cache: false
                        }

                        Label {
                            text: qsTr("Open code: %1").arg(openCode)
                            font.family: Theme.fontFamilyHeading
                            color: Theme.highlightColor
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                bottom: image.top
                                bottomMargin: Theme.paddingMedium
                            }
                        }
                    }
                }

                SectionHeader {
                    id: sectionHeader
                    anchors.top: parent.top
                    anchors.topMargin: Theme.paddingMedium
                    topPadding: 0
                    height: implicitHeight
                    text: shipmentNumber
                    font.pixelSize: Theme.fontSizeMedium
                }

                Image {
                    anchors {
                        bottom: dataGrid.bottom
                        right: dataGrid.right
                        rightMargin: Theme.paddingLarge
                        bottomMargin: Theme.paddingSmall
                    }

                    width: Theme.iconSizeLarge
                    height: Theme.iconSizeLarge
                    source: "qrc:///images/icon-m-parcel-locker-" + parcelSize + "-var.svg"
                    sourceSize.width: width
                    sourceSize.height: height
                }

                Row {
                    id: dataGrid
                    anchors {
                        top: parent.top
                        topMargin: sectionHeader.height
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }

                    spacing: -Theme.paddingMedium

                    Column {
                        id: labelColumn
                        width: parent.width*0.25
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            elide: Text.ElideRight
                            width: parent.width - Theme.horizontalPageMargin
                            leftPadding: Theme.horizontalPageMargin
                            horizontalAlignment: Text.AlignRight
                            truncationMode: TruncationMode.Fade
                            text: parcelOwnership === 2 ? qsTr("Ownership") : listTypeSelect.currentIndex === 2 ? qsTr("Receiver:") : qsTr("Sender:")
                        }

                        Label {
                            elide: Text.ElideRight
                            width: parent.width - Theme.horizontalPageMargin
                            leftPadding: Theme.horizontalPageMargin
                            horizontalAlignment: Text.AlignRight
                            truncationMode: TruncationMode.Fade
                            text: qsTr("Status:")
                        }

                        Label {
                            elide: Text.ElideRight
                            width: parent.width - Theme.horizontalPageMargin
                            leftPadding: Theme.horizontalPageMargin
                            horizontalAlignment: Text.AlignRight
                            truncationMode: TruncationMode.Fade
                            text: qsTr("Type:")
                        }
                    }

                    Column {
                        height: labelColumn.height
                        width: parent.width*0.75
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            elide: Text.ElideRight
                            width: parent.width - Theme.horizontalPageMargin
                            rightPadding: Theme.horizontalPageMargin
                            color: Theme.secondaryColor
                            truncationMode: TruncationMode.Fade
                            text: parcelOwnership === 2 ? qsTr("Observed") : senderName
                        }

                        Label {
                            elide: Text.ElideRight
                            width: parent.width - Theme.horizontalPageMargin
                            rightPadding: Theme.horizontalPageMargin
                            color: Theme.secondaryColor
                            truncationMode: TruncationMode.Fade
                            text: parcelStatus
                        }

                        Label {
                            elide: Text.ElideRight
                            width: parent.width - Theme.horizontalPageMargin
                            rightPadding: Theme.horizontalPageMargin
                            color: Theme.secondaryColor
                            truncationMode: TruncationMode.Fade
                            text: parcelType
                        }
                    }
                }
            }
            VerticalScrollDecorator {}
        }
    }
}
