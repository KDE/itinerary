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
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    property var arrival: _liveDataManager.arrival(batchId)
    property var departure: _liveDataManager.departure(batchId)

    headerIconSource: departure.route.line.mode == Line.Unknown ? "qrc:///images/train.svg" : PublicTransport.lineModeIcon(departure.route.line.mode)
    headerItem: RowLayout {
        QQC2.Label {
            text: {
                if (reservationFor.trainName || reservationFor.trainNumber) {
                    return reservationFor.trainName + " " + reservationFor.trainNumber
                }
                return i18n("%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
            }
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservationFor, "departureTime")
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        }
        QQC2.Label {
            text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
            color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departure.hasExpectedDepartureTime
        }
    }

    contentItem: ColumnLayout {
        id: topLayout

        QQC2.Label {
            text: i18n("Departure from %1 on platform %2",
                reservationFor.departureStation.name,
                departure.hasExpectedPlatform ? departure.expectedPlatform :
                reservationFor.departurePlatform ? reservationFor.departurePlatform : "-")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        App.PlaceDelegate {
            place: reservationFor.departureStation
            Layout.fillWidth: true
        }
        // TODO reserved seat

        Kirigami.Separator {
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("Arrival at %1 on platform %2",
                reservationFor.arrivalStation.name,
                arrival.hasExpectedPlatform ? arrival.expectedPlatform :
                reservationFor.arrivalPlatform ? reservationFor.arrivalPlatform : "-")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.fillWidth: true
            QQC2.Label {
                text: i18n("Arrival time: %1", Localizer.formatDateTime(reservationFor, "arrivalTime"))
                color: Kirigami.Theme.textColor
                wrapMode: Text.WordWrap
                visible: reservationFor.arrivalTime.length > 0
            }
            QQC2.Label {
                text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: arrival.hasExpectedArrivalTime
            }
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalStation
            Layout.fillWidth: true
        }
    }

    Component {
        id: detailsComponent
        App.TrainPage {
            batchId: root.batchId
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}
