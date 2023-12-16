// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.itinerary

Components.FloatingButton {
    id: root

    required property Kirigami.Page page

    icon.name: "view-barcode-qr"
    text: i18nc("@action:button", "Barcode Scan Mode")
    onClicked: scanModeController.toggle()
    checkable: true
    checked: scanModeController.enabled

    anchors {
        right: parent.right
        rightMargin: Kirigami.Units.largeSpacing + (page.contentItem.QQC2.ScrollBar && page.contentItem.QQC2.ScrollBar.vertical ? page.contentItem.QQC2.ScrollBar.vertical.width : 0)
        bottom: parent.bottom
        bottomMargin: Kirigami.Units.largeSpacing
    }

    BarcodeScanModeController {
        id: scanModeController
        page: root.page
    }
}
