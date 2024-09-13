/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitinerary
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    /** The reservation batch identifier (@see ReservationManager). */
    property alias batchId: _controller.batchId
    /** Currently selected reservation id of the batch. */
    property var currentReservationId: batchId
    /** Currently selected reservation of the batch. */
    property var reservation: ReservationManager.reservation(currentReservationId);
    /** Reservation::reservationFor, unique for all travelers on a multi-traveler reservation set */
    readonly property var reservationFor: reservation.reservationFor
    property Component editor
    readonly property var reservationIds: ReservationManager.reservationsForBatch(root.batchId)

    function seatString(): string {
        let s = [];
        for (const reservationId of root.reservationIds) {
            const reservation = ReservationManager.reservation(reservationId);
            const seat = reservation?.reservedTicket?.ticketedSeat;
            if (seat) {
                s.push(seat.seatNumber);
            }
        }
        if (s.length === 0) {
            return "-";
        }
        return s.join(", ");
    }



    /** @c true if we have at least one seat reserverion in this batch. */
    readonly property bool hasSeat: {
        for (const resId of reservationIds) {
            const res = ReservationManager.reservation(resId);
            const seat = res?.reservedTicket?.ticketedSeat;
            if (seat && seat.seatNumber !== "")
                return true;
        }
        return false;
    }

    property QtObject controller: TimelineDelegateController {
        id: _controller
        reservationManager: ReservationManager
        liveDataManager: LiveDataManager
        transferManager: TransferManager
        documentManager: DocumentManager
        tripGroupManager: TripGroupManager
    }
    property alias arrival: _controller.arrival
    property alias departure: _controller.departure

    leftPadding: 0
    rightPadding: 0

    Connections {
        target: ReservationManager
        function onBatchContentChanged(batchId) {
            if (batchId == root.batchId) {
                root.reservation = Qt.binding(function() { return ReservationManager.reservation(root.currentReservationId); })
            }
        }
    }
}
