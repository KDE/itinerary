/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Bus Reservation")

    property var departureBusStop: reservationFor.departureBusStop
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalBusStop: reservationFor.arrivalBusStop
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    function save(resId, reservation) {
        var trip = reservation.reservationFor;
        trip.departureBusStop = root.departureBusStop;
        trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
        trip.departurePlatform = departurePlatform.text;
        trip.arrivalBusStop = root.arrivalBusStop;
        trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
        trip.arrivalPlatform = arrivalPlatform.text;

        var newRes = reservation;
        newRes.reservationFor = trip;

        ReservationManager.updateReservation(resId, newRes);
    }

    App.IntermediateStopSelector {
        id: boardSheet
        title: i18n("Board Later")
        model: root.controller.journey.intermediateStops
        action: Kirigami.Action {
            text: i18n("Change departure stop")
            onTriggered: {
                departureBusStop = PublicTransport.busStationFromLocation(root.controller.journey.intermediateStops[boardSheet.currentIndex].stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[boardSheet.currentIndex], "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    App.IntermediateStopSelector {
        id: alightSheet
        title: i18n("Alight Earlier")
        model: root.controller.journey.intermediateStops
        action: Kirigami.Action {
            text: i18n("Change arrival stop")
            onTriggered: {
                arrivalBusStop = PublicTransport.busStationFromLocation(root.controller.journey.intermediateStops[alightSheet.currentIndex].stopPoint);
                arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightSheet.currentIndex], "scheduledArrivalTime");
                if (!arrivalTime) {
                    arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightSheet.currentIndex], "scheduledDepartureTime");
                }
                alightSheet.close();
            }
        }
    }

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: reservationFor.busNumber || reservationFor.busName
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: reservationFor.busName + " " + reservationFor.busNumber
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18nc("bus departure", "Departure")
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("bus stop", "Stop")
                    description: root.departureBusStop.name
                }
                MobileForm.FormTextFieldDelegate {
                    id: departurePlatform
                    label: i18nc("bus stop platform", "Platform")
                    text: reservationFor.departurePlatform
                }
                MobileForm.FormButtonDelegate {
                    text: i18n("Board later")
                    icon.name: "arrow-right"
                    visible: root.controller.journey && root.controller.journey.intermediateStops.length > 0 // TODO also check for preceding layovers
                    onClicked: boardSheet.open();
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18nc("bus arrival", "Arrival")
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("bus stop", "Stop")
                    description: root.arrivalBusStop.name
                }
                MobileForm.FormTextFieldDelegate {
                    id: arrivalPlatform
                    label: i18nc("bus stop platform", "Platform")
                    text: reservationFor.arrivalPlatform
                }
                MobileForm.FormButtonDelegate {
                    text: i18n("Alight earlier")
                    icon.name: "arrow-left"
                    visible: root.controller.journey && root.controller.journey.intermediateStops.length > 0 // TODO also check for subsequent layovers
                    onClicked: alightSheet.open();
                }
            }
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Booking")
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextDelegate {
                    text: i18n("Reference")
                    description: reservation.reservationNumber
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextDelegate {
                    text: i18n("Under name")
                    description: reservation.underName.name
                }
            }
        }
    }
}
