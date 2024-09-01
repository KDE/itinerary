/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: "Development Mode"

    FileDialog {
        id: mapcssDialog
        title: "Import MapCSS"
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        nameFilters: ["MapCSS style sheets (*.mapcss)"]
        onAccepted: DevelopmentModeController.importMapCSS(selectedFile)
    }

    Component {
        id: tripGroupsPage
        TripGroupsPage {}
    }

    ColumnLayout {
        QQC2.Button {
            text: "Disable Development Mode"
            Layout.fillWidth: true
            onClicked: {
                Settings.developmentMode = false;
                showPassiveNotification("Development mode disabled");
                applicationWindow().pageStack.goBack();
            }
        }

        QQC2.Button {
            text: "Import MapCSS"
            Layout.fillWidth: true
            onClicked: mapcssDialog.open()
        }
        QQC2.Button {
            text: "Reset MapCSS"
            Layout.fillWidth: true
            onClicked: DevelopmentModeController.purgeMapCSS();
        }
        QQC2.Button {
            text: "Clear OSM Tile Cache"
            Layout.fillWidth: true
            onClicked: DevelopmentModeController.clearOsmTileCache();
        }

        QQC2.Button {
            text: "Enable KPT Logging"
            Layout.fillWidth: true
            onClicked: DevelopmentModeController.enablePublicTransportLogging();
        }

        QQC2.Button {
            text: "Crash"
            Layout.fillWidth: true
            onClicked: DevelopmentModeController.crash();
        }

        QQC2.Label {
            text: DevelopmentModeController.screenInfo();
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.family: "monospace"
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    Clipboard.saveText(DevelopmentModeController.screenInfo());
                    applicationWindow().showPassiveNotification("Screen information copied");
                }
            }
        }

        QQC2.Label {
            text: DevelopmentModeController.localeInfo();
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.family: "monospace"
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    Clipboard.saveText(DevelopmentModeController.localeInfo());
                    applicationWindow().showPassiveNotification("Locale information copied");
                }
            }
        }
    }
}
