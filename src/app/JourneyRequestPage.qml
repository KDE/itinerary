/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.dateandtime 0.1 as Addon
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    property var publicTransportManager

    /**
     * Pre-selected country in the location pickers.
     * If not specified the country from the current locale is used.
     */
    property string initialCountry
    /** Pre-selected departure time. */
    property date initialDateTime: new Date()

    property var departureStop
    property var arrivalStop

    title: i18n("Select Journey")

    actions.main: Kirigami.Action {
        icon.name: "search"
        text: i18n("Search Journey")
        enabled: root.departureStop != undefined && root.arrivalStop != undefined
        onTriggered: {
            applicationWindow().pageStack.push(journeyQueryPage);
            var req = applicationWindow().pageStack.currentItem.journeyRequest;
            req.from = root.departureStop;
            req.to = root.arrivalStop;

            const dt = new Date(dateInput.selectedDate.getFullYear(), dateInput.selectedDate.getMonth(), dateInput.selectedDate.getDate(), timeInput.value.getHours(), timeInput.value.getMinutes());
            console.log(dt, dateInput.selectedDate, timeInput.value);
            req.dateTime = dt;
            req.maximumResults = 6;
            console.log(req);
            applicationWindow().pageStack.currentItem.journeyRequest = req;
        }
    }

    Component {
        id: departurePicker
        StopPickerPage {
            title: i18nc("departure train station", "Select Departure Stop")
            publicTransportManager: root.publicTransportManager
            initialCountry: root.initialCountry
            // force a deep copy, otherwise this breaks as soon as the other stop picker page is shown...
            onLocationChanged: root.departureStop = PublicTransport.copyLocation(location);
        }
    }
    Component {
        id: arrivalPicker
        StopPickerPage {
            title: i18nc("arrival train station", "Select Arrival Stop")
            publicTransportManager: root.publicTransportManager
            initialCountry: root.initialCountry
            onLocationChanged: root.arrivalStop = PublicTransport.copyLocation(location)
        }
    }
    Component {
        id: journeyQueryPage
        JourneyQueryPage {
            publicTransportManager: root.publicTransportManager
            title: i18n("Select Journey")
            onJourneyChanged: {
                for (const section of journey.sections) {
                    if (section.mode != JourneySection.PublicTransport) {
                        continue;
                    }
                    const res = PublicTransport.reservationFromJourneySection(section);
                    const resId = ReservationManager.addReservation(res);
                    LiveDataManager.setJourney(resId, section);
                }
                applicationWindow().pageStack.pop();
                applicationWindow().pageStack.pop();
            }
        }
    }

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            text: i18nc("departure train station", "From:")
        }
        QQC2.Button {
            Layout.fillWidth: true
            text: departureStop ? departureStop.name : i18nc("departure train station", "Select Departure Stop")
            onClicked: applicationWindow().pageStack.push(departurePicker)
        }
        QQC2.Label {
            text: i18nc("arrival train station", "To:")
        }
        QQC2.Button {
            Layout.fillWidth: true
            text: arrivalStop ? arrivalStop.name : i18nc("arrival train station", "Select Arrival Stop")
            onClicked: applicationWindow().pageStack.push(arrivalPicker)
        }
        QQC2.Label {
            text: i18nc("departure time for a train", "Departure time:")
        }
        RowLayout {
            Layout.fillWidth: true
            Addon.DateInput {
                id: dateInput
                selectedDate: root.initialDateTime
            }
            Addon.TimeInput {
                id: timeInput
                value: root.initialDateTime
            }
        }
    }
}
