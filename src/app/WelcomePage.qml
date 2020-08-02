/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtPositioning 5.11
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: ReservationManager.isEmpty() ? i18n("Welcome!") : i18n("Help")

    ColumnLayout {
        Kirigami.Heading {
            text: i18n("How to import data?")
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("<p>There's a number of ways to import data into KDE Itinerary:<ul>
                <li>Directly opening PDF tickets or Apple Wallet passes.</li>
                <li>From the Android calendar, for entries made via the KMail, Nextcloud or Thunderbird Itinerary plugins, and synced using DavDroid.</li>
                <li>Via KDE Connect from the KMail Itinerary plugin.</li>
                <li>By scanning boarding pass barcodes and pasting their content.</li>
                </p>")
            wrapMode: Text.WordWrap
        }

        Kirigami.Heading {
            text: i18n("Check the settings!")
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("KDE Itinerary has all features disabled by default that require online access, such as retrieving live traffic data or weather forecasts. You therefore might want to review these settings. While you are at it, you might want to configure your home location to enable the transfer assistant to automatically suggests ways to your next departure station or airport.")
            wrapMode: Text.WordWrap
        }

        Kirigami.Heading {
            text: i18n("More information")
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("For more information about KDE Itinerary check out the <a href=\"https://community.kde.org/KDE_PIM/KDE_Itinerary\">wiki page</a>.")
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }


        QQC2.Button {
            text: i18n("Got it!")
            onClicked: applicationWindow().pageStack.pop();
            Layout.alignment: Qt.AlignRight
            visible: ReservationManager.isEmpty()
        }
    }
}
