/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.itinerary 1.0
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
            name: "OpenStreetMap"
            copyright: "© OpenStreetMap contributors"
            licenseId: "ODbL-1.0"
            licenseName: "Open Data Commons Open Database License"
            url: "https://www.openstreetmap.org/"
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
            name: "Poppler"
            licenseId: "GPL-2.0-or-later"
            licenseName: "GNU General Public License v2 or later"
            url: "https://poppler.freedesktop.org/"
        }
        ListElement {
            name: "FreeType"
            copyright: "Copyright (C) 2006-2020 by David Turner, Robert Wilhelm, and Werner Lemberg."
            platform: "android"
            licenseId: "FTL"
            licenseName: "GNU General Public License v2 or Freetype Project License"
            url: "https://www.freetype.org/"
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
            highlighted: false
            Column {
                id: layout
                QQC2.Label {
                    text: model.name
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
                QQC2.Label {
                    text: i18n("Copyright: %1", model.copyright)
                    visible: model.copyright != undefined
                    wrapMode: Text.WordWrap
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
                QQC2.Label {
                    text: i18n("Homepage: <a href=\"%1\">%1</a>", model.url)
                    visible: model.url != undefined
                    onLinkActivated: Qt.openUrlExternally(link)
                    wrapMode: Text.WordWrap
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
                QQC2.Label {
                    text: i18n("License: <a href=\"https://spdx.org/licenses/%1.html\">%2</a>", model.licenseId, model.licenseName)
                    wrapMode: Text.WordWrap
                    onLinkActivated: Qt.openUrlExternally(link)
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }
        }
    }

    Component {
        id: transportDataDelegate
        Kirigami.AbstractListItem {
            width: ListView.view.width
            highlighted: false
            ColumnLayout {
                QQC2.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "<a href=\"" + modelData.url + "\">" + modelData.name + "</a>"
                    onLinkActivated: Qt.openUrlExternally(link)
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18n("License: <a href=\"%2\">%1</a>", (modelData.license != "" ? modelData.license : modelData.licenseUrl), modelData.licenseUrl)
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
        QQC2.TabButton { text: i18n("Version") }
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
                model: LiveDataManager.publicTransportManager.attributions
                spacing: 0
                delegate: transportDataDelegate
            }
        }

        ColumnLayout {
            id: versionPage
            width: parent.width
            height: parent.height

            QQC2.Label {
                id: versionLabel
                Layout.fillWidth: true
                text: i18n("Application version: %1", ApplicationController.version)

                // developement mode activation
                property int tapCount: 0
                TapHandler {
                    onTapped: {
                        versionLabel.tapCount++;
                        if (versionLabel.tapCount == 7) {
                            Settings.developmentMode = true;
                            showPassiveNotification("Development mode enabled!");
                        }
                    }
                }
            }

            Kirigami.Separator { Layout.fillWidth: true }

            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("Extractor capabilities:")
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: ApplicationController.extractorCapabilities
                font.family: "monospace"
            }
        }
    }
}
