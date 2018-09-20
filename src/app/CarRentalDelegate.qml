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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
            text: root.rangeType == TimelineModel.RangeEnd ?
                i18n("Rental Car Drop-off") :
                i18n("Rental Car Pick-up")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: root.rangeType ==  TimelineModel.RangeEnd ?
                Localizer.formatTime(reservation, "dropoffTime") :
                Localizer.formatTime(reservation, "pickupTime")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            color: Kirigami.Theme.textColor
        }
    }

    contentItem: ColumnLayout {
        id: topLayout

        QQC2.Label {
            text: reservation.pickupLocation.name
            visible: root.rangeType != TimelineModel.RangeEnd
        }
        App.PlaceDelegate {
            place: reservation.pickupLocation
            Layout.fillWidth: true
            visible: root.rangeType != TimelineModel.RangeEnd
        }
        QQC2.Label {
            text: i18n("Drop-off: %1", Localizer.formatDateTime(reservation, "dropoffTime"))
        }
        QQC2.Label {
            text: reservation.dropoffLocation.name
        }
        App.PlaceDelegate {
            place: reservation.dropoffLocation
            Layout.fillWidth: true
        }
    }

    Component {
        id: detailsComponent
        App.CarRentalPage {
            resIds: root.resIds
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}

