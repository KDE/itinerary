/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

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
    title: i18n("Edit Train Reservation")

    property var departureStation: reservationFor.departureStation
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalStation: reservationFor.arrivalStation
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    function save(resId, reservation) {
        var trip = reservation.reservationFor;
        trip.departureStation = root.departureStation;
        trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
        trip.departurePlatform = departurePlatform.text;
        trip.arrivalStation = root.arrivalStation;
        trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
        trip.arrivalPlatform = arrivalPlatform.text;

        var seat = reservation.reservedTicket.ticketedSeat;
        seat.seatSection = coachEdit.text;
        seat.seatNumber = seatEdit.text;
        seat.seatingType = classEdit.text;

        var ticket = reservation.reservedTicket;
        ticket.ticketedSeat = seat;

        var newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;

        ReservationManager.updateReservation(resId, newRes);
    }

    Component {
        id: intermediateStopDelegate
        Kirigami.BasicListItem {
            text: {
                if (modelData.scheduledDepartureTime.getTime()) {
                    return Localizer.formatTime(modelData, "scheduledDepartureTime") + " " + modelData.stopPoint.name
                }
                return Localizer.formatTime(modelData, "scheduledArrivalTime") + " " + modelData.stopPoint.name
            }
            enabled: modelData.disruptionEffect != Disruption.NoService
        }
    }

    Kirigami.OverlaySheet {
        id: boardSheet
        header: Kirigami.Heading {
            text: i18n("Board Later")
        }

        ListView {
            id: boardStopSelector
            model: root.controller.journey.intermediateStops
            delegate: intermediateStopDelegate
        }

        footer: QQC2.Button {
            text: i18n("Change departure station")
            enabled: boardStopSelector.currentIndex >= 0
            onClicked: {
                departureStation = PublicTransport.trainStationFromLocation(root.controller.journey.intermediateStops[boardStopSelector.currentIndex].stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightStopSelector.currentIndex], "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    Kirigami.OverlaySheet {
        id: alightSheet
        header: Kirigami.Heading {
            text: i18n("Alight Earlier")
        }

        ListView {
            id: alightStopSelector
            model: root.controller.journey.intermediateStops
            delegate: intermediateStopDelegate
        }

        footer: QQC2.Button {
            text: i18n("Change arrival station")
            enabled: alightStopSelector.currentIndex >= 0
            onClicked: {
                arrivalStation = PublicTransport.trainStationFromLocation(root.controller.journey.intermediateStops[alightStopSelector.currentIndex].stopPoint);
                arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightStopSelector.currentIndex], "scheduledArrivalTime");
                if (!arrivalTime) {
                    arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightStopSelector.currentIndex], "scheduledDepartureTime");
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
            visible: reservationFor.trainNumber || reservationFor.trainName
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: reservationFor.trainName + " " + reservationFor.trainNumber
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
                    title: i18nc("train departure", "Departure")
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("train station", "Station")
                    description: root.departureStation.name
                }
                MobileForm.FormTextFieldDelegate {
                    id: departurePlatform
                    label: i18nc("train platform", "Platform")
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
                    title: i18nc("train arrival", "Arrival")
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("train station", "Station")
                    description: root.arrivalStation.name
                }
                MobileForm.FormTextFieldDelegate {
                    id: arrivalPlatform
                    label: i18nc("train platform", "Platform")
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
                    title: i18n("Seat")
                }
                MobileForm.FormTextFieldDelegate {
                    id: coachEdit
                    label: i18nc("carriage on a train", "Coach")
                    text: reservation.reservedTicket.ticketedSeat.seatSection
                }
                MobileForm.FormTextFieldDelegate {
                    id: seatEdit
                    label: i18n("Seat")
                    text: reservation.reservedTicket.ticketedSeat.seatNumber
                }
                MobileForm.FormTextFieldDelegate {
                    id: classEdit
                    label: i18nc("seating class on a train", "Class")
                    text: reservation.reservedTicket.ticketedSeat.seatingType
                }
            }
        }

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
