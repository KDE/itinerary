/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormCard {
    id: root
    width: ListView.view.width
    /** Reservation batch identifier (@see ReservationManager). */
    property alias batchId: _controller.batchId
    /** All reservations that are part of this patch. */
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)
    /** A random reservation object, in case there's more than one.
     *  Use this only for accessing properties that will be the same for all travelers.
     */
    readonly property alias reservation: _controller.reservation
    /** Reservation::reservationFor, unique for all travelers on a multi-traveler reservation set */
    readonly property var reservationFor: reservation.reservationFor
    property var rangeType

    /** Details page to show when clicking the delegate. */
    property Component detailsPage: null

    property alias contentItem: content.contentItem
    signal clicked

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

    onClicked: {
        if (!root.detailsPage || !root.batchId) {
            return;
        }
        while (applicationWindow().pageStack.depth > 1 && !applicationWindow().pageStack.currentItem as TripGroupPage) {
            applicationWindow().pageStack.pop();
        }
        applicationWindow().pageStack.push(root.detailsPage, { batchId: root.batchId });
    }

    /** @c true if we have at least one seat reserverion in this batch. */
    readonly property bool hasSeat: {
        for (const resId of resIds) {
            const res = ReservationManager.reservation(resId);
            const seat = res?.reservedTicket?.ticketedSeat;
            if (seat && seat.seatNumber !== "")
                return true;
        }
        return false;
    }

    function seatSectionString(): string {
        let s = []
        for (const resId of resIds) {
            const res = ReservationManager.reservation(resId);
            const seat = res?.reservedTicket?.ticketedSeat;
            if (seat && seat.seatSection)
                s.push(seat.seatSection);
        }
        if (s.length === 0)
            return "-";
        return [...new Set(s)].join(", ");
    }

    function seatString(): string {
        let s = []
        for (const resId of resIds) {
            const res = ReservationManager.reservation(resId);
            const seat = res?.reservedTicket?.ticketedSeat;
            if (seat && seat.seatNumber)
                s.push(seat.seatNumber);
        }
        if (s.length === 0)
            return "-";
        return s.join(", ");
    }

    FormCard.AbstractFormDelegate {
        id: content
        onClicked: root.clicked()
        Layout.fillWidth: true
    }

    Accessible.onPressAction: root.clicked()
}
