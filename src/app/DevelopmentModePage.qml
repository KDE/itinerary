/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.1
import org.kde.kirigami 2.12 as Kirigami
import org.kde.itinerary 1.0

Kirigami.ScrollablePage {
    id: root
    title: "Development Mode"

    FileDialog {
        id: mapcssDialog
        title: "Import MapCSS"
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: ["MapCSS style sheets (*.mapcss)"]
        onAccepted: DevelopmentModeController.importMapCSS(file)
    }

    ColumnLayout {
        QQC2.Button {
            text: "Disable Development Mode"
            Layout.fillWidth: true
            onClicked: {
                Settings.developmentMode = false;
                showPassiveNotification("Development mode disabled");
                applicationWindow().pageStack.pop();
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
    }
}
