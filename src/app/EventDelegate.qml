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

    headerIconSource: "meeting-attending"
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: root.rangeType == TimelineElement.RangeEnd ?
                i18n("End: %1", reservationFor.name) : reservationFor.name
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
            elide: Text.ElideRight
            Layout.fillWidth: true
            Accessible.ignored: true
        }
        QQC2.Label {
            text: {
                if (root.rangeType == TimelineElement.RangeEnd)
                    return Localizer.formatTime(reservationFor, "endDate");
                if (reservationFor.doorTime > 0)
                    return Localizer.formatTime(reservationFor, "doorTime");
                return Localizer.formatTime(reservationFor, "startDate");
            }
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: reservationFor.location != undefined ? reservationFor.location.name : ""
            visible: text !== ""
            wrapMode: Text.WordWrap
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.location
            controller: root.controller
            isRangeBegin: root.rangeType == TimelineElement.RangeBegin
            isRangeEnd: root.rangeType == TimelineElement.RangeEnd
            width: topLayout.width
            visible: reservationFor.location != undefined
            showButtons: false
        }
        QQC2.Label {
            text: i18n("Start time: %1", Localizer.formatDateTime(reservationFor, "startDate"))
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.doorTime > 0
        }
        QQC2.Label {
            text: i18n("End time: %1", Localizer.formatDateTime(reservationFor, "endDate"));
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.endDate > 0
        }
    }

    onClicked: showDetailsPage(eventDetailsPage, root.batchId)

    Accessible.name: headerLabel.text
}
