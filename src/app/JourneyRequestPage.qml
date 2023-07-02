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

    // either true/false if all mode switches are in that position, undefined otherwise
    function fullModeSwitchState()
    {
        let state = longDistanceSwitch.checked;
        for (const s of [localTrainSwitch, rapidTransitSwitch, busSwitch, ferrySwitch]) {
            if (s.checked != state) {
                return undefined;
            }
        }
        return state;
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
                    enabled: root.departureStop != undefined && root.arrivalStop != undefined && root.fullModeSwitchState() !== false
                    onClicked: {
                        applicationWindow().pageStack.push(journeyQueryPage);
                        const req = applicationWindow().pageStack.currentItem.journeyRequest;
                        req.from = root.departureStop;
                        req.to = root.arrivalStop;

                        const dt = new Date(dateInput.selectedDate.getFullYear(), dateInput.selectedDate.getMonth(), dateInput.selectedDate.getDate(), timeInput.value.getHours(), timeInput.value.getMinutes());
                        console.log(dt, dateInput.selectedDate, timeInput.value);
                        req.dateTime = dt;
                        req.maximumResults = 6;
                        req.downloadAssets = true;

                        let lineModes = [];
                        if (root.fullModeSwitchState() == undefined) {
                            if (longDistanceSwitch.checked)
                                lineModes.push(Line.LongDistanceTrain, Line.Train);
                            if (localTrainSwitch.checked)
                                lineModes.push(Line.LocalTrain);
                            if (rapidTransitSwitch.checked)
                                lineModes.push(Line.RapidTransit, Line.Metro, Line.Tramway, Line.RailShuttle);
                            if (busSwitch.checked)
                                lineModes.push(Line.Bus, Line.Coach);
                            if (ferrySwitch.checked)
                                lineModes.push(Line.Ferry, Line.Boat);
                        }
                        req.lineModes = lineModes;

                        console.log(req);
                        applicationWindow().pageStack.currentItem.journeyRequest = req;
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormSwitchDelegate {
                    id: longDistanceSwitch
                    text: i18nc("journey query search constraint, title", "Long distance trains")
                    description: i18nc("journey query search constraint, description", "High speed or intercity trains")
                    checked: true
                    leading: Kirigami.Icon {
                        source: PublicTransport.lineModeIcon(Line.LongDistanceTrain)
                        isMask: true
                    }
                }
                MobileForm.FormSwitchDelegate {
                    id: localTrainSwitch
                    text: i18nc("journey query search constraint, title", "Local trains")
                    description: i18nc("journey query search constraint, description", "Regional or local trains")
                    checked: true
                    leading: Kirigami.Icon {
                        source: PublicTransport.lineModeIcon(Line.LocalTrain)
                        isMask: true
                    }
                }
                MobileForm.FormSwitchDelegate {
                    id: rapidTransitSwitch
                    text: i18nc("journey query search constraint, title", "Rapid transit")
                    description: i18nc("journey query search constraint, description", "Rapid transit, metro, trams")
                    checked: true
                    leading: Kirigami.Icon {
                        source: PublicTransport.lineModeIcon(Line.Tramway)
                        isMask: true
                    }
                }
                MobileForm.FormSwitchDelegate {
                    id: busSwitch
                    text: i18nc("journey query search constraint, title", "Bus")
                    description: i18nc("journey query search constraint, description", "Local or regional bus services")
                    checked: true
                    leading: Kirigami.Icon {
                        source: PublicTransport.lineModeIcon(Line.Bus)
                        isMask: true
                    }
                }
                MobileForm.FormSwitchDelegate {
                    id: ferrySwitch
                    text: i18nc("journey query search constraint, title", "Ferry")
                    description: i18nc("journey query search constraint, description", "Boats or ferries")
                    checked: true
                    leading: Kirigami.Icon {
                        source: PublicTransport.lineModeIcon(Line.Ferry)
                        isMask: true
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
