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

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
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
                source: "qrc:///images/bus.svg"
                width: Kirigami.Units.iconSizes.smallMedium
                height: width
                color: Kirigami.Theme.textColor
                isMask: true
            }

            QQC2.Label {
                text: reservationFor.busName + " " + reservationFor.busNumber
                color: Kirigami.Theme.textColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: Localizer.formatTime(reservationFor, "departureTime")
                color: Kirigami.Theme.textColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            }
        }
    }

    contentItem: ColumnLayout {
        id: topLayout

        QQC2.Label {
            text: i18n("From: %1", reservationFor.departureStation.name)
            color: Kirigami.Theme.textColor
        }
        App.PlaceDelegate {
            place: reservationFor.departureStation
            Layout.fillWidth: true
        }
        Kirigami.Separator {
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("To: %1", reservationFor.arrivalStation.name)
            color: Kirigami.Theme.textColor
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalStation
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("Arrival: %1", Localizer.formatDateTime(reservationFor, "arrivalTime"))
            color: Kirigami.Theme.textColor
        }
    }

    Component {
        id: detailsComponent
        App.BusPage {
            resIds: root.resIds
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}

