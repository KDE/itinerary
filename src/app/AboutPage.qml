/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AboutPage {
    id: root

    title: i18n("About")

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
        title: i18n("Developer information")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            id: versionLabel
            text: i18n("Application version: %1", ApplicationController.version)

            // development mode activation
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
