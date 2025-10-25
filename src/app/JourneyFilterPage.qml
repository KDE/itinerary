/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.itinerary
import org.kde.kpublictransport

FormCard.FormCardPage {
    id: root
    title: i18nc("@title", "Filter Journeys")

    property var initialLineModes: []

    readonly property var longDistanceModes: [Line.LongDistanceTrain, Line.Train]
    readonly property var localTrainModes: [Line.LocalTrain]
    readonly property var rapidTransitModes: [Line.RapidTransit, Line.Metro, Line.Tramway, Line.RailShuttle, Line.Funicular, Line.AerialLift]
    readonly property var busModes: [Line.Bus, Line.Coach]
    readonly property var ferryModes: [Line.Ferry, Line.Boat]
    readonly property var aircraftModes: [Line.Air]

    readonly property var lineModes: {
        var modes = [];
        if (root.fullModeSwitchState() == undefined) {
            if (longDistanceSwitch.checked)
                modes.push(...root.longDistanceModes);
            if (localTrainSwitch.checked)
                modes.push(...root.localTrainModes);
            if (rapidTransitSwitch.checked)
                modes.push(...root.rapidTransitModes);
            if (busSwitch.checked)
                modes.push(...root.busModes);
            if (ferrySwitch.checked)
                modes.push(...root.ferryModes);
            if (aircraftSwitch.checked)
                modes.push(...root.aircraftModes);
        }
        modes
    }

    // either true/false if all mode switches are in that position, undefined otherwise
    function fullModeSwitchState()
    {
        let state = root.longDistance;
        for (const s of [localTrainSwitch, rapidTransitSwitch, busSwitch,
                         ferrySwitch, aircraftSwitch]) {
            if (s.checked != state) {
                return undefined;
            }
        }
        return state;
    }

    FormCard.FormHeader {
        title: i18n("Mode of transportation")
    }

    function checkLineModes(modes: var): bool {
        return modes.every((mode) => root.initialLineModes.indexOf(mode) != -1)
    }

    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            id: longDistanceSwitch
            text: i18nc("journey query search constraint, title", "Long distance trains")
            description: i18nc("journey query search constraint, description", "High speed or intercity trains")
            checked: root.checkLineModes(root.longDistanceModes)
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.LongDistanceTrain)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: localTrainSwitch
            text: i18nc("journey query search constraint, title", "Local trains")
            description: i18nc("journey query search constraint, description", "Regional or local trains")
            checked: root.checkLineModes(root.localTrainModes)
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.LocalTrain)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: rapidTransitSwitch
            text: i18nc("journey query search constraint, title", "Rapid transit")
            description: i18nc("journey query search constraint, description", "Rapid transit, metro, trams")
            checked: root.checkLineModes(root.rapidTransitModes)
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Tramway)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: busSwitch
            text: i18nc("journey query search constraint, title", "Bus")
            description: i18nc("journey query search constraint, description", "Local or regional bus services")
            checked: root.checkLineModes(root.busModes)
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Bus)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: ferrySwitch
            text: i18nc("journey query search constraint, title", "Ferry")
            description: i18nc("journey query search constraint, description", "Boats or ferries")
            checked: root.checkLineModes(root.ferryModes)
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Ferry)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: aircraftSwitch
            text: i18nc("journey query search constraint, title", "Airplane")
            checked: root.checkLineModes(root.aircraftModes)
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Air)
                isMask: true
            }
        }
    }
}
