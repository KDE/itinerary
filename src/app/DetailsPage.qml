/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import internal.org.kde.kcalendarcore 1.0 as KCalendarCore
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    /** The reservation batch identifier (@see ReservationManager). */
    property alias batchId: _controller.batchId
    /** Currently selected reservation id of the batch. */
    property var currentReservationId: batchId
    /** Currently selected reservation of the batch. */
    readonly property var currentReservation: ReservationManager.reservation(currentReservationId);
    /** @deprecated */
    readonly property var reservation: ReservationManager.reservation(currentReservationId);
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
}
