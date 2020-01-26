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

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root

    headerIconSource: "meeting-attending"
    headerItem: RowLayout {
        QQC2.Label {
            text: root.rangeType == TimelineElement.RangeEnd ?
                i18n("End: %1", reservationFor.name) : reservationFor.name
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
            elide: Text.ElideRight
            Layout.fillWidth: true
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

    contentItem: ColumnLayout {
        id: topLayout

        QQC2.Label {
            text: reservationFor.location.name
        }
        App.PlaceDelegate {
            place: reservationFor.location
            controller: root.controller
            isRangeBegin: root.rangeType == TimelineElement.RangeBegin
            isRangeEnd: root.rangeType == TimelineElement.RangeEnd
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("Start time: %1", Localizer.formatDateTime(reservationFor, "startDate"))
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.doorTime > 0
        }
        QQC2.Label {
            text: i18n("End time: %1", Localizer.formatDateTime(reservationFor, "endDate"));
            visible: root.rangeType != TimelineElement.RangeEnd
        }
    }

    Component {
        id: detailsComponent
        App.EventPage {
            batchId: root.batchId
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}
