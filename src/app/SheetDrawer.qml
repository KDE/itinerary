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
    property Component footerItem

    signal closed()
    Component.onCompleted: {
        sheet.closed.connect(root.closed)
        drawer.closed.connect(root.closed)

    }

    function open() {
        if (Kirigami.Settings.isMobile) {
            drawer.open()
            drawer.drawerContentItem = contentItem.createObject(drawer)
            if (root.headerItem)
                drawer.headerContentItem = headerItem.createObject(drawer)
        } else {
            sheet.open()
            sheet.contentItem = contentItem.createObject(sheet)
            if (root.headerItem)
                sheet.header.contentItem = headerItem.createObject(sheet)
            if (root.footerItem)
                sheet.footer.contentItem = footerItem.createObject(sheet)
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


    QQC2.Dialog {
        id: sheet
        parent: applicationWindow().overlay
        background: Components.DialogRoundedBackground {}
        header: QQC2.Control {
            leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: 0
        }
        footer: QQC2.Control {
            leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: 0
        }

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 30)
        height: Math.min(parent.height - Kirigami.Units.gridUnit * 4, implicitHeight)

        rightPadding: 0
        leftPadding: 0
        bottomPadding: 0

    }

}
