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
    headerIconSource: departure.route.line.mode == Line.Unknown ? "qrc:///images/bus.svg" : PublicTransport.lineModeIcon(departure.route.line.mode)
    headerItem: RowLayout {
        QQC2.Label {
            text: {
                if (reservationFor.busName || reservationFor.busNumber ) {
                    return reservationFor.busName + " " + reservationFor.busNumber
                }
                return i18n("%1 to %2", reservationFor.departureBusStop.name, reservationFor.arrivalBusStop.name);
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
            text: i18n("From: %1", reservationFor.departureBusStop.name)
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
        App.PlaceDelegate {
            place: reservationFor.departureBusStop
            currentLocation: root.previousLocation
            Layout.fillWidth: true
        }
        Kirigami.Separator {
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("To: %1", reservationFor.arrivalBusStop.name)
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalBusStop
            offerNavigation: false
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.fillWidth: true
            QQC2.Label {
                text: i18n("Arrival time: %1", Localizer.formatDateTime(reservationFor, "arrivalTime"))
                color: Kirigami.Theme.textColor
                wrapMode: Text.WordWrap
                visible: reservationFor.arrivalTime > 0
            }
            QQC2.Label {
                text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: arrival.hasExpectedArrivalTime
            }
        }
    }

    Component {
        id: detailsComponent
        App.BusPage {
            batchId: root.batchId
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}

