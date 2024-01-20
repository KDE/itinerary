/*
    SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components

Item {
    id: root
    property Component contentItem
    property Component headerItem
    function open() {
        if(Kirigami.Settings.isMobile) {
            drawer.open()
            drawer.drawerContentItem = contentItem.createObject(drawer)
            drawer.headerContentItem = headerItem.createObject(drawer)


        } else {
            sheet.open()
            sheet.flickableContentData = [contentItem.createObject(sheet)]
            sheet.header = headerItem.createObject(sheet)

        }
    }
    function close() {
        drawer.close()
        sheet.close()
    }

    Components.BottomDrawer {
        id: drawer
        parent: applicationWindow().overlay
    }


    Kirigami.OverlaySheet {
        id: sheet
        parent: applicationWindow().overlay
        width: Kirigami.Units.gridUnit * 29
        height: Kirigami.Units.gridUnit * 15

        rightPadding: 0
        leftPadding: 0

    }
}
