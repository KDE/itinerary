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
    property alias headerIcon: _headerIcon
    property alias headerIconSource: _headerIcon.source

    property double headerFontScale: 1.0

    showClickFeedback: true

    property QtObject controller: TimelineDelegateController {
        id: _controller
        reservationManager: ReservationManager
        liveDataManager: LiveDataManager
        transferManager: TransferManager
        documentManager: DocumentManager
    }
    property alias arrival: _controller.arrival
    property alias departure: _controller.departure

    function showDetails(detailsComponent) {
        while (applicationWindow().pageStack.depth > 1) {
            applicationWindow().pageStack.pop();
        }
        applicationWindow().pageStack.push(detailsComponent);
    }

    header: TimelineDelegateHeaderBackground {
        id: headerBackground
        card: root
        Kirigami.Theme.colorSet: controller.isCurrent ? Kirigami.Theme.Selection : Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        defaultColor: {
            if (controller.connectionWarning)
                return Kirigami.Theme.negativeTextColor;
            if (controller.isCanceled)
                return Kirigami.Theme.disabledTextColor;
            return Kirigami.Theme.backgroundColor;
        }
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                id: _headerIcon
                Layout.preferredWidth: Layout.preferredHeight * aspectRatio
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                color: Kirigami.Theme.textColor
                isMask: true

                // work around the fact that Kirigami.Icon always gives us an implicit size of 32x32 no
                // matter the size of the input icon
                property real aspectRatio: isMask ? 1.0 : Util.svgAspectRatio(source)
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

    Accessible.onPressAction: root.clicked()
}
