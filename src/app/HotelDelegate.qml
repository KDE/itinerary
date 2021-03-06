/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root

    headerIconSource: "go-home-symbolic"
    headerItem: QQC2.Label {
        text: root.rangeType == TimelineElement.RangeEnd ?
            i18n("Check-out %1", reservationFor.name) : reservationFor.name
        color: Kirigami.Theme.textColor
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        elide: Text.ElideRight
        Layout.fillWidth: true
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        App.PlaceDelegate {
            place: reservationFor
            controller: root.controller
            width: topLayout.width
            isRangeBegin: root.rangeType == TimelineElement.RangeBegin
            isRangeEnd: root.rangeType == TimelineElement.RangeEnd
        }
        QQC2.Label {
            text: i18n("Check-in time: %1", Localizer.formatTime(reservation, "checkinTime"))
            color: Kirigami.Theme.textColor
            visible: root.rangeType == TimelineElement.RangeBegin
        }
        QQC2.Label {
            text: root.rangeType == TimelineElement.RangeBegin ?
                i18n("Check-out time: %1", Localizer.formatDateTime(reservation, "checkoutTime")) :
                i18n("Check-out time: %1", Localizer.formatTime(reservation, "checkoutTime"))
            color: Kirigami.Theme.textColor
        }

    }

    Component {
        id: detailsComponent
        App.HotelPage {
            batchId: root.batchId
        }
    }

    onClicked: showDetails(detailsComponent)
}
