/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary
import "." as App

import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AboutPage {
    id: root

    title: i18n("About")

    aboutData: About

    data: ListModel {
        id: licenseModel
        ListElement {
            name: "KDE Itinerary, KDE PIM and KDE Frameworks 6"
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
        ListElement {
            name: "libical"
            copyright: "© The libical developers"
            platform: "android"
            licenseId: "LGPL-2.1-only"
            licenseName: "GNU Lesser General Public License v2.1"
            url: "https://github.com/libical/libical"
        }
        ListElement {
            name: "libxml2"
            copyright: "Copyright (C) 1998-2012 Daniel Veillard"
            platform: "android"
            licenseId: "MIT"
            licenseName: "MIT License"
            url: "https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home"
        }
        ListElement {
            name: "libQuotient"
            platform: "android"
            licenseId: "LGPL-2.1-or-later"
            licenseName: "GNU Lesser General Public License v2.1 or later"
            url: "https://github.com/frankosterfeld/qtkeychain/"
        }
        ListElement {
            name: "QtKeychain"
            platform: "android"
            licenseId: "BSD-3-Clause"
            licenseName: "Modified BSD License"
            url: "https://github.com/frankosterfeld/qtkeychain/"
        }
        ListElement {
            name: "Olm"
            copyright: "OpenMarket Ltd"
            platform: "android"
            licenseId: "Apache-2.0"
            licenseName: "Apache License 2.0"
            url: "https://gitlab.matrix.org/matrix-org/olm"
        }
    }

    property Component licenseDelegate: FormCard.AbstractFormDelegate {
        visible: !model.platform || model.platform == Qt.platform.os
        background: null

        contentItem: ColumnLayout {
            id: layout
            QQC2.Label {
                text: model.name
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: model.copyright != undefined ? i18n("Copyright: %1", model.copyright) : ""
                visible: model.copyright != undefined
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: model.url != undefined ? i18n("Homepage: <a href=\"%1\">%1</a>", model.url) : ""
                visible: model.url != undefined
                onLinkActivated: Qt.openUrlExternally(link)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: i18n("License: <a href=\"https://spdx.org/licenses/%1.html\">%2</a>", model.licenseId, model.licenseName)
                wrapMode: Text.WordWrap
                onLinkActivated: Qt.openUrlExternally(link)
                Layout.fillWidth: true
            }
        }
    }

    property Component transportDataDelegate: FormCard.AbstractFormDelegate {
        background: null
        contentItem: ColumnLayout {
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

    FormCard.FormHeader {
        title: i18n("KDE Itinerary is developed by the KDE community as free software and uses the components listed below.")
    }

    FormCard.FormCard {
        Repeater {
            model: licenseModel
            delegate: licenseDelegate
        }
    }

    FormCard.FormHeader {
        title: i18n("KDE Itinerary uses public transport data from the following sources.")
    }

    FormCard.FormCard {
        Repeater {
            model: LiveDataManager.publicTransportManager.attributions
            delegate: transportDataDelegate
        }
    }

    FormCard.FormHeader {
        title: i18n("Developer information")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            id: versionLabel
            text: i18n("Application version: %1", ApplicationController.version)

            // developement mode activation
            property int tapCount: 0
            onClicked: {
                versionLabel.tapCount++;
                if (versionLabel.tapCount == 7) {
                    Settings.developmentMode = true;
                    showPassiveNotification("Development mode enabled!");
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            text: i18n("Extractor capabilities:")
            description: ApplicationController.extractorCapabilities
            font.family: "monospace"
            onClicked: {
                Clipboard.saveText(ApplicationController.extractorCapabilities);
                applicationWindow().showPassiveNotification(i18n("Extractor capabilities copied to clipboard"));
            }
        }
    }
}
