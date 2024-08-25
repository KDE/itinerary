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

    property QtObject controller: TimelineDelegateController {
        id: _controller
        reservationManager: ReservationManager
        liveDataManager: LiveDataManager
        transferManager: TransferManager
        documentManager: DocumentManager
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
