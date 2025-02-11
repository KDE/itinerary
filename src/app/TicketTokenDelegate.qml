/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.kde.prison as Prison
import org.kde.kitinerary
import org.kde.itinerary

ColumnLayout {
    id: root
    property var resIds

    readonly property var currentReservationId: ticketModel.reservationIdAt(travelerBox.currentIndex)
    readonly property var currentTicket: ticketModel.reservationAt(travelerBox.currentIndex) ? ticketModel.reservationAt(travelerBox.currentIndex).reservedTicket : undefined
    readonly property int ticketTokenCount: travelerBox.count
    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing
    Layout.bottomMargin: Kirigami.Units.largeSpacing

    /** There is a barcode displayed. */
    readonly property alias hasBarcode: barcodeContainer.visible
    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    TicketTokenModel {
        id: ticketModel
        reservationManager: ReservationManager
        reservationIds: resIds
    }

    QQC2.ComboBox {
        id: travelerBox
        Layout.alignment: Qt.AlignCenter
        model: ticketModel
        textRole: "display"
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing

        // ugly, but rowCount does not trigger binding changes
        Component.onCompleted: {
            visible = ticketModel.rowCount() >= 1 && root.resIds.length > 1;
            currentIndex = ticketModel.initialIndex;
        }
    }

    BarcodeContainer {
        id: barcodeContainer
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        barcodeType: currentTicket ? currentTicket.ticketTokenType : Ticket.Unknown
        barcodeContent: currentTicket ? currentTicket.ticketTokenData : undefined
        onDoubleClicked: root.scanModeToggled()
    }
}
