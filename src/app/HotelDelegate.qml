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

    headerIconSource: "go-home"
    headerItem: QQC2.Label {
        text: root.rangeType == TimelineModel.RangeEnd ?
            i18n("Check-out %1", reservationFor.name) : reservationFor.name
        color: Kirigami.Theme.textColor
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        Layout.fillWidth: true
    }

    contentItem: ColumnLayout {
        id: topLayout

        App.PlaceDelegate {
            place: reservationFor
            currentLocation: root.previousLocation
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("Check-in time: %1", Localizer.formatTime(reservation, "checkinTime"))
            color: Kirigami.Theme.textColor
            visible: root.rangeType == TimelineModel.RangeBegin
        }
        QQC2.Label {
            text: root.rangeType == TimelineModel.RangeBegin ?
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

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}
