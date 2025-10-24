/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.dateandtime as Addon
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    property KPublicTransport.Manager publicTransportManager

    /**
     * Pre-selected country in the location pickers.
     * If not specified the country from the current locale is used.
     */
    property string initialCountry
    /** Pre-selected departure time. */
    property date initialDateTime: new Date()

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    property var departureStop
    property var arrivalStop

    title: i18n("Select Journey")

    Component {
        id: tripGroupDialogComponent

        TripGroupSelectorDialog {
            id: tripGroupDialog
            property var journey

            onTripGroupSelected: (tgId) => {
                for (const section of journey.sections) {
                    if (section.mode != JourneySection.PublicTransport) {
                        continue;
                    }
                    const res = PublicTransport.reservationFromJourneySection(section);
                    const resId = ApplicationController.addNewReservation(res, tgId);
                    LiveDataManager.setJourney(resId, section);
                }
                ApplicationController.contextTripGroupId = tgId;
                tripGroupDialog.close()
                pageStack.clear()
                pageStack.push(pagepool.loadPage(Qt.resolvedUrl("TripGroupsPage.qml")))
            }
        }
    }

    data: [
        Component {
            id: departurePicker
            StopPickerPage {
                title: i18nc("departure train station", "Select Departure Stop")
                publicTransportManager: root.publicTransportManager
                initialCountry: root.initialCountry
                // force a deep copy, otherwise this breaks as soon as the other stop picker page is shown...
                onLocationChanged: root.departureStop = PublicTransport.copyLocation(location);
                historySortRoleName: Settings.read("StopPicker/historySortMode", "lastUsed")
                onHistorySortRoleChanged: (sortRoleName) => Settings.write("StopPicker/historySortMode", sortRoleName)
                showUseCurrentLocationButton: true
            }
        },
        Component {
            id: arrivalPicker
            StopPickerPage {
                title: i18nc("arrival train station", "Select Arrival Stop")
                publicTransportManager: root.publicTransportManager
                initialCountry: root.initialCountry
                onLocationChanged: root.arrivalStop = PublicTransport.copyLocation(location)
                historySortRoleName: Settings.read("StopPicker/historySortMode", "lastUsed")
                onHistorySortRoleChanged: (sortRoleName) => Settings.write("StopPicker/historySortMode", sortRoleName)
            }
        },
        Component {
            id: journeyQueryPage
            JourneyQueryPage {
                id: queryPage

                publicTransportManager: root.publicTransportManager
                title: i18n("Select Journey")
                onJourneyChanged: {
                    let tripGroupDialog = tripGroupDialogComponent.createObject();
                    tripGroupDialog.journey = queryPage.journey
                    tripGroupDialog.suggestedName = arrivalStop ? arrivalStop.name : ""
                    tripGroupDialog.open()
                }
            }
        }
    ]

    // either true/false if all mode switches are in that position, undefined otherwise
    function fullModeSwitchState()
    {
        let state = longDistanceSwitch.checked;
        for (const s of [localTrainSwitch, rapidTransitSwitch, busSwitch, ferrySwitch, aircraftSwitch]) {
            if (s.checked != state) {
                return undefined;
            }
        }
        return state;
    }

    FormCard.FormCard {
        id: requestCard
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: fromButton

            text: i18nc("departure train station", "From:")
            description: departureStop ? departureStop.name : i18nc("departure train station", "Select Departure Stop")
            onClicked: applicationWindow().pageStack.push(departurePicker)
        }


        FormCard.FormDelegateSeparator {
            below: fromButton
            above: toButton
        }

        FormCard.FormButtonDelegate {
            id: toButton

            text: i18nc("arrival train station", "To:")
            description: arrivalStop ? arrivalStop.name : i18nc("arrival train station", "Select Arrival Stop")
            onClicked: applicationWindow().pageStack.push(arrivalPicker)
        }
        Item{
            width:parent.width
            height: 0
            QQC2.RoundButton{
                icon.name: "reverse"
                y: -fromButton.height - height/2
                z: toButton.z + 10000
                x: fromButton.width - width/2 - Kirigami.Units.gridUnit *3
                onClicked:{
                    var oldDepartureStop = departureStop
                    departureStop = arrivalStop
                    arrivalStop = oldDepartureStop
                }

                Accessible.name: i18nc("departure and arrival stops", "Swap departure and arrival")
            }
        }
        FormCard.FormDelegateSeparator {
            below: toButton
            above: departureArrivalSelector
        }

        FormCard.FormRadioSelectorDelegate {
            id: departureArrivalSelector

            consistentWidth: true

            actions: [
                Kirigami.Action {
                    text: i18nc("train or bus departure", "Departure")
                },
                Kirigami.Action {
                    text: i18nc("train or bus arrival", "Arrival")
                }
            ]
        }

        FormCard.FormDelegateSeparator {
            below: departureArrivalSelector
            above: dateTimeInput
        }

        FormCard.FormDateTimeDelegate {
            id: dateTimeInput
            value: root.initialDateTime
            Accessible.name: departureArrivalSelector.selectedIndex === 0 ? i18nc("train or bus departure", "Departure time") : i18nc("train or bus arrival", "Arrival time")
        }

        FormCard.FormDelegateSeparator {
            below: dateTimeInput
            above: searchButton
        }

        FormCard.FormButtonDelegate {
            id: searchButton
            icon.name: "search"
            text: i18n("Search Journey")
            enabled: root.departureStop != undefined && root.arrivalStop != undefined && root.fullModeSwitchState() !== false
            onClicked: {
                applicationWindow().pageStack.push(journeyQueryPage);
                const req = applicationWindow().pageStack.currentItem.journeyRequest;
                req.from = root.departureStop;
                req.to = root.arrivalStop;

                console.log(dateTimeInput.value);
                req.dateTime = dateTimeInput.value;
                req.maximumResults = 6;
                req.downloadAssets = Settings.wikimediaOnlineContentEnabled;
                req.includePaths = true;
                // TODO rental vehicle support
                req.modes = JourneySection.PublicTransport | JourneySection.Walking;

                let lineModes = [];
                if (root.fullModeSwitchState() == undefined) {
                    if (longDistanceSwitch.checked)
                        lineModes.push(Line.LongDistanceTrain, Line.Train);
                    if (localTrainSwitch.checked)
                        lineModes.push(Line.LocalTrain);
                    if (rapidTransitSwitch.checked)
                        lineModes.push(Line.RapidTransit, Line.Metro, Line.Tramway, Line.RailShuttle, Line.Funicular, Line.AerialLift);
                    if (busSwitch.checked)
                        lineModes.push(Line.Bus, Line.Coach);
                    if (ferrySwitch.checked)
                        lineModes.push(Line.Ferry, Line.Boat);
                    if (aircraftSwitch.checked)
                        lineModes.push(Line.Air);
                }
                req.lineModes = lineModes;

                if (departureArrivalSelector.selectedIndex === 0) {
                    req.dateTimeMode = JourneyRequest.Departure
                } else if (departureArrivalSelector.selectedIndex === 1) {
                    req.dateTimeMode = JourneyRequest.Arrival
                }

                console.log(req);
                applicationWindow().pageStack.currentItem.journeyRequest = req;
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Mode of transportation")
    }

    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            id: longDistanceSwitch
            text: i18nc("journey query search constraint, title", "Long distance trains")
            description: i18nc("journey query search constraint, description", "High speed or intercity trains")
            checked: true
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.LongDistanceTrain)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: localTrainSwitch
            text: i18nc("journey query search constraint, title", "Local trains")
            description: i18nc("journey query search constraint, description", "Regional or local trains")
            checked: true
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.LocalTrain)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: rapidTransitSwitch
            text: i18nc("journey query search constraint, title", "Rapid transit")
            description: i18nc("journey query search constraint, description", "Rapid transit, metro, trams")
            checked: true
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Tramway)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: busSwitch
            text: i18nc("journey query search constraint, title", "Bus")
            description: i18nc("journey query search constraint, description", "Local or regional bus services")
            checked: true
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Bus)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: ferrySwitch
            text: i18nc("journey query search constraint, title", "Ferry")
            description: i18nc("journey query search constraint, description", "Boats or ferries")
            checked: true
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Ferry)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: aircraftSwitch
            text: i18nc("journey query search constraint, title", "Airplane")
            checked: false
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Air)
                isMask: true
            }
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
