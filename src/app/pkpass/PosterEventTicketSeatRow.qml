/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass

/*! Seat information fields of poster event tickets. */
GridLayout {
    id: root

    /*! The pass these seat is from. */
    property KPkPass.Pass pass: null
    /*! The seat to show information for. */
    property KPkPass.seat seat
    /*! Fallback text color when not specified in the pass. */
    property color defaultTextColor: palette.text

    rows: 2
    columns: 4
    Layout.fillWidth: true

    // TODO only show those fields actually populated
    // TODO font size for the bottom row
    QQC2.Label {
        text: i18nc("seat reservation", "Aisle")
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        font: Kirigami.Theme.smallFont
        color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
    }
    QQC2.Label {
        text: i18nc("seat reservation", "Section")
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        font: Kirigami.Theme.smallFont
        color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
    }
    QQC2.Label {
        text: i18nc("seat reservation", "Row")
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        font: Kirigami.Theme.smallFont
        color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
    }
    QQC2.Label {
        text: i18nc("seat reservation", "Seat")
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        font: Kirigami.Theme.smallFont
        color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
    }

    QQC2.Label {
        text: root.seat.seatAisle
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
    }
    QQC2.Label {
        text: root.seat.seatSection
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
    }
    QQC2.Label {
        text: root.seat.seatRow
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
    }
    QQC2.Label {
        text: root.seat.seatNumber
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
    }
}
