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

    property Kirigami.Page requestPage

    Component.onCompleted: {
        root.requestPage.longDistance = Qt.binding(() => longDistanceSwitch.checked)
        root.requestPage.localTrain = Qt.binding(() => localTrainSwitch.checked)
        root.requestPage.rapidTransit = Qt.binding(() => rapidTransitSwitch.checked)
        root.requestPage.bus = Qt.binding(() => busSwitch.checked)
        root.requestPage.ferry = Qt.binding(() => ferrySwitch.checked)
        root.requestPage.aircraft = Qt.binding(() => aircraftSwitch.checked)
    }

    FormCard.FormHeader {
        title: i18n("Mode of transportation")
    }

    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            id: longDistanceSwitch
            text: i18nc("journey query search constraint, title", "Long distance trains")
            description: i18nc("journey query search constraint, description", "High speed or intercity trains")
            checked: root.requestPage.longDistance
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.LongDistanceTrain)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: localTrainSwitch
            text: i18nc("journey query search constraint, title", "Local trains")
            description: i18nc("journey query search constraint, description", "Regional or local trains")
            checked: root.requestPage.localTrain
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.LocalTrain)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: rapidTransitSwitch
            text: i18nc("journey query search constraint, title", "Rapid transit")
            description: i18nc("journey query search constraint, description", "Rapid transit, metro, trams")
            checked: root.requestPage.rapidTransit
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Tramway)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: busSwitch
            text: i18nc("journey query search constraint, title", "Bus")
            description: i18nc("journey query search constraint, description", "Local or regional bus services")
            checked: root.requestPage.bus
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Bus)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: ferrySwitch
            text: i18nc("journey query search constraint, title", "Ferry")
            description: i18nc("journey query search constraint, description", "Boats or ferries")
            checked: root.requestPage.ferry
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Ferry)
                isMask: true
            }
        }
        FormCard.FormSwitchDelegate {
            id: aircraftSwitch
            text: i18nc("journey query search constraint, title", "Airplane")
            checked: root.requestPage.aircraft
            leading: Kirigami.Icon {
                source: LineMode.iconName(Line.Air)
                isMask: true
            }
        }
    }
}
