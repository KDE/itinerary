/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Window 2.10
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

ColumnLayout {
    id: root
    property var resIds

    readonly property var currentReservationId: ticketModel.reservationIdAt(travelerBox.currentIndex)
    readonly property var currentTicket: ticketModel.reservationAt(travelerBox.currentIndex) ? ticketModel.reservationAt(travelerBox.currentIndex).reservedTicket : undefined
    Layout.fillWidth: true

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
        barcodeType: currentTicket ? currentTicket.ticketTokenType : undefined
        barcodeContent: currentTicket ? currentTicket.ticketTokenData : undefined
        onDoubleClicked: root.scanModeToggled()
    }
}
