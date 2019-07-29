/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

import QtQuick 2.10
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import "." as App

Kirigami.Page {
    id: root
    title: i18n("About")

    ListModel {
        id: licenseModel
        ListElement {
            name: "KDE Itinerary, KDE PIM and KDE Frameworks 5"
            copyright: "© The KDE community"
            licenseId: "LGPL-2.0-or-later"
            licenseName: "GNU Library General Public License v2 or later"
            url: "https://www.kde.org/"
        }
        ListElement {
            name: "Qt"
            copyright: "© The Qt Project"
            licenseId: "LGPL-3.0-only"
            licenseName: "GNU Lesser General Public License v3.0 only"
        }
        ListElement {
            name: "Wikidata"
            licenseId: "CC0-1.0"
            licenseName: "Creative Commons Zero v1.0 Universal"
            url: "https://www.wikidata.org/"
        }
        ListElement {
            name: "OpenSSL"
            copyright: "© 1998-2018 The OpenSSL Project"
            licenseId: "OpenSSL"
            licenseName: "OpenSSL License"
            url: "https://www.openssl.org/"
        }
        ListElement {
            name: "ZXing C++"
            copyright: "© 2016 ZXing Authors, © 2016 Nu-book Inc."
            licenseId: "Apache-2.0"
            licenseName: "Apache License 2.0"
            url: "https://github.com/nu-book/zxing-cpp"
        }
        ListElement {
            name: "iCal4j"
            copyright: "© 2012, Ben Fortuna"
            platform: "android"
            licenseId: "BSD-3-Clause"
            licenseName: "BSD 3-Clause License"
            url: "https://github.com/ical4j/ical4j"
        }
        ListElement {
            name: "libintl lite"
            platform: "android"
            licenseId: "BSL-1.0"
            licenseName: "Boost Software License 1.0"
            url: "https://github.com/j-jorge/libintl-lite"
        }
        ListElement {
            name: "libqrencode"
            copyright: "© 2006-2018 Kentaro Fukuchi"
            platform: "android"
            licenseId: "LGPL-2.1-or-later"
            licenseName: "GNU Lesser General Public License v2.1 or later"
            url: "https://github.com/fukuchi/libqrencode"
        }
    }

    Component {
        id: licenseDelegate
        Kirigami.AbstractListItem {
            visible: !model.platform || model.platform == Qt.platform.os
            height: visible ? implicitHeight : 0
            width: ListView.view.width
            ColumnLayout {
                id: layout
                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.name
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18n("Copyright: %1", model.copyright)
                    visible: model.copyright != undefined
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18n("Homepage: <a href=\"%1\">%1</a>", model.url)
                    visible: model.url != undefined
                    onLinkActivated: Qt.openUrlExternally(link)
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18n("License: <a href=\"https://spdx.org/licenses/%1.html\">%2</a>", model.licenseId, model.licenseName)
                    wrapMode: Text.WordWrap
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }
    }

    Component {
        id: transportDataDelegate
        Kirigami.AbstractListItem {
            width: ListView.view.width
            ColumnLayout {
                QQC2.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "<a href=\"" + modelData.url + "\">" + modelData.name + "</a>"
                    onLinkActivated: Qt.openUrlExternally(link)
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18n("License: <a href=\"%2\">%1</a>", modelData.license, modelData.licenseUrl)
                    onLinkActivated: Qt.openUrlExternally(link)
                    visible: modelData.hasLicense
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    header: QQC2.TabBar {
        id: tabBar
        QQC2.TabButton { text: i18n("Application") }
        QQC2.TabButton { text: i18n("Transport Data") }
    }

    StackLayout {
        id: tabLayout
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        ColumnLayout {
            id: appPage
            width: parent.width
            height: parent.height

            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("KDE Itinerary is developed by the KDE community as free software and uses the components listed below.")
                wrapMode: Text.WordWrap
            }

            Kirigami.Separator { Layout.fillWidth: true }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: contentHeight

                boundsMovement: Flickable.StopAtBounds
                clip: true
                model: licenseModel
                spacing: 0
                delegate: licenseDelegate
            }
        }

        ColumnLayout {
            id: transportDataPage
            width: parent.width
            height: parent.height

            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("KDE Itinerary uses public transport data from the following sources.")
                wrapMode: Text.WordWrap
            }

            Kirigami.Separator { Layout.fillWidth: true }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: contentHeight

                boundsMovement: Flickable.StopAtBounds
                clip: true
                model: _liveDataManager.publicTransportManager.attributions
                spacing: 0
                delegate: transportDataDelegate
            }
        }
    }
}
