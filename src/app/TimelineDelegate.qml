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
import "." as App

FormCard.FormCard {
    id: root
    width: parent.width
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
    property alias contentItem: content.contentItem
    signal clicked

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

    property color headerTextColor: controller.isCanceled ? headerBackground.Kirigami.Theme.disabledTextColor : headerBackground.Kirigami.Theme.textColor
    FormCard.AbstractFormDelegate {
        onClicked: root.clicked()
        background: Rectangle {
            id: headerBackground
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: controller.isCurrent ? Kirigami.Theme.Selection : controller.isCanceled ? Kirigami.Theme.View : Kirigami.Theme.Header
            Kirigami.Theme.inherit: false

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
        contentItem: RowLayout {
            id: headerLayout

            Kirigami.Icon {
                id: _headerIcon
                Layout.preferredWidth: Layout.preferredHeight * aspectRatio
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                color: root.headerTextColor
                isMask: true

                // work around the fact that Kirigami.Icon always gives us an implicit size of 32x32 no
                // matter the size of the input icon
                property real aspectRatio: isMask ? 1.0 : Util.svgAspectRatio(source)
            }
        }
    }

    FormCard.AbstractFormDelegate {
        id: content
        onClicked: root.clicked()
        Layout.fillWidth: true

    }

    onHeaderItemChanged: {
        if (headerItem) {
            headerItem.parent = headerLayout
        }
    }

    Accessible.onPressAction: root.clicked()
}
