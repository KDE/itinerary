/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    id: root
    /** Reservation batch identifier (@see _reservationManager). */
    property alias batchId: controller.batchId
    /** All reservations that are part of this patch. */
    property var resIds: _reservationManager.reservationsForBatch(root.batchId)
    /** A random reservation object, in case there's more than one.
     *  Use this only for accessing properties that will be the same for all travelers.
     */
    readonly property var reservation: _reservationManager.reservation(root.batchId);
    /** Reservation::reservationFor, unique for all travelers on a multi-traveler reservation set */
    readonly property var reservationFor: reservation.reservationFor
    property var rangeType

    property Item headerItem
    property string headerIconSource

    readonly property double headerFontScale: 1.25

    showClickFeedback: true

    TimelineDelegateController {
        id: controller
        reservationManager: _reservationManager
        liveDataManager: _liveDataManager
    }

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: controller.isCurrent ? Kirigami.Theme.Selection : Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
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
                width: Kirigami.Units.iconSizes.smallMedium
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
