/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kirigamiaddons.dateandtime 0.1 as Addon
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
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
    leftPadding: 0
    rightPadding: 0

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
        anchors.fill: parent

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    id: fromButton

                    text: i18nc("departure train station", "From:")
                    description: departureStop ? departureStop.name : i18nc("departure train station", "Select Departure Stop")
                    onClicked: applicationWindow().pageStack.push(departurePicker)
                }

                MobileForm.FormDelegateSeparator {
                    below: fromButton
                    above: toButton
                }

                MobileForm.FormButtonDelegate {
                    id: toButton

                    text: i18nc("arrival train station", "To:")
                    description: arrivalStop ? arrivalStop.name : i18nc("arrival train station", "Select Arrival Stop")
                    onClicked: applicationWindow().pageStack.push(arrivalPicker)
                }

                MobileForm.FormDelegateSeparator {
                    below: toButton
                    above: timeSelector
                }

                MobileForm.AbstractFormDelegate {
                    id: timeSelector
                    text: i18nc("departure time for a train", "Departure time:")

                    contentItem: ColumnLayout {
                        QQC2.Label {
                            text: timeSelector.text
                            Layout.fillWidth: true
                        }

                        Flow {
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

                MobileForm.FormDelegateSeparator {
                    below: timeSelector
                    above: searchButton
                }

                MobileForm.FormButtonDelegate {
                    id: searchButton
                    icon.name: "search"
                    text: i18n("Search Journey")
                    enabled: root.departureStop != undefined && root.arrivalStop != undefined
                    onClicked: {
                        applicationWindow().pageStack.push(journeyQueryPage);
                        const req = applicationWindow().pageStack.currentItem.journeyRequest;
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
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
