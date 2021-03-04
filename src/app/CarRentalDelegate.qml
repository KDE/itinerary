/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root

    headerIconSource: "qrc:///images/car.svg"
    headerItem: RowLayout {
        QQC2.Label {
            text: root.rangeType == TimelineElement.RangeEnd ?
                i18n("Rental Car Drop-off") :
                i18n("Rental Car Pick-up")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: root.rangeType ==  TimelineElement.RangeEnd ?
                Localizer.formatTime(reservation, "dropoffTime") :
                Localizer.formatTime(reservation, "pickupTime")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: reservation.pickupLocation.name
            visible: root.rangeType != TimelineElement.RangeEnd
        }
        App.PlaceDelegate {
            place: reservation.pickupLocation
            controller: root.controller
            isRangeBegin: true
            width: topLayout.width
            visible: root.rangeType != TimelineElement.RangeEnd
        }
        QQC2.Label {
            text: i18n("Drop-off: %1", Localizer.formatDateTime(reservation, "dropoffTime"))
            visible: root.rangeType != TimelineElement.RangeEnd
        }
        QQC2.Label {
            text: reservation.dropoffLocation.name
            visible: root.rangeType != TimelineElement.RangeBegin
        }
        App.PlaceDelegate {
            place: reservation.dropoffLocation
            isRangeEnd: true
            width: topLayout.width
            visible: root.rangeType != TimelineElement.RangeBegin
            controller: root.controller
        }
    }

    Component {
        id: detailsComponent
        App.CarRentalPage {
            batchId: root.batchId
        }
    }

    onClicked: showDetails(detailsComponent)
}

