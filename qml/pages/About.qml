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
import "../modules/Opal/About" as A

A.AboutPageBase {
    id: page

    appName: "Outpost"
    appIcon: "/usr/share/icons/hicolor/172x172/apps/outpost.png"
    appVersion: "1.2"
    description: qsTr("Outpost is an unofficial inpost app")

    authors: ["Michał Szczepaniak"]
    mainAttributions: []
    sourcesUrl: "https://github.com/Michal-Szczepaniak/Outpost"

    licenses: A.License {
        spdxId: "GPL-3.0"
    }

    donations.text: donations.defaultTextGeneral
    donations.services: [
        A.DonationService {
            name: "Paypal"
            url: "https://www.paypal.me/MisterMagister"
        }
    ]

    contributionSections: [
        A.ContributionSection {
            title: qsTr("Development")
            groups: [
                A.ContributionGroup {
                    title: qsTr("Programming")
                    entries: ["Michał Szczepaniak"]
                },
                A.ContributionGroup {
                    title: qsTr("Icon Design")
                    entries: ["gregguh"]
                }
            ]
        },
        A.ContributionSection {
            title: qsTr("Translations")
            groups: [
                A.ContributionGroup {
                    title: qsTr("English")
                    entries: ["Michał Szczepaniak"]
                }
            ]
        }
    ]
}
