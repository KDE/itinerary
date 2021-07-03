/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    id: root
    /** Reservation batch identifier (@see ReservationManager). */
    property alias batchId: _controller.batchId
    /** All reservations that are part of this patch. */
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)
    /** A random reservation object, in case there's more than one.
     *  Use this only for accessing properties that will be the same for all travelers.
     */
    readonly property var reservation: ReservationManager.reservation(root.batchId);
    /** Reservation::reservationFor, unique for all travelers on a multi-traveler reservation set */
    readonly property var reservationFor: reservation.reservationFor
    property var rangeType

    property Item headerItem
    property string headerIconSource

    readonly property double headerFontScale: 1.0

    showClickFeedback: true

    property QtObject controller: TimelineDelegateController {
        id: _controller
        reservationManager: ReservationManager
        liveDataManager: LiveDataManager
        transferManager: TransferManager
    }
    property alias arrival: _controller.arrival
    property alias departure: _controller.departure

    function showDetails(detailsComponent) {
        while (applicationWindow().pageStack.depth > 1) {
            applicationWindow().pageStack.pop();
        }
        applicationWindow().pageStack.push(detailsComponent);
    }

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: controller.isCurrent ? Kirigami.Theme.Selection : Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: {
            if (controller.connectionWarning)
                return Kirigami.Theme.negativeTextColor;
            if (controller.isCanceled)
                return Kirigami.Theme.disabledTextColor;
            return Kirigami.Theme.backgroundColor;
        }
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.rightMargin: -root.rightPadding

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: headerIconSource
                width: Kirigami.Units.iconSizes.small
                height: width
                color: Kirigami.Theme.textColor
                isMask: true
            }
        }

        Rectangle {
            id: progressBar
            visible: controller.isCurrent
            anchors.bottom: headerBackground.bottom
            anchors.left: headerBackground.left
            height: Kirigami.Units.smallSpacing
            width: controller.progress * headerBackground.width
            color: Kirigami.Theme.visitedLinkColor
        }
    }

    onHeaderItemChanged: {
        if (headerItem) {
            headerItem.parent = headerLayout
        }
    }
}
