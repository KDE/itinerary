/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import internal.org.kde.kcalendarcore 1.0 as KCalendarCore
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root

    property alias calendar: importModel.calendar

    title: i18n("Calendar Import")

    actions.main: Kirigami.Action {
        icon.name: "document-open"
        text: i18n("Import selected events")
        enabled: importModel.hasSelection
        onTriggered: {
            importModel.selectedReservations().forEach(r => ReservationManager.importReservation(r));
            applicationWindow().pageStack.pop();
        }
    }

    CalendarImportModel {
        id: importModel
    }

    ListView {
        id: eventList
        model: importModel

        delegate: Kirigami.AbstractListItem {
            highlighted: model.selected
            GridLayout {
                columns: 2
                rows: 2

                Kirigami.Icon {
                    Layout.rowSpan: 2
                    isMask: true
                    source: model.iconName
                }
                QQC2.Label {
                    text: model.title
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                QQC2.Label {
                    text: model.subtitle
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font: Kirigami.Theme.smallFont
                }
            }
            onClicked: model.selected = !model.selected
        }

        Kirigami.PlaceholderMessage {
            text: i18n("No importable events found in this caledar.")
            visible: eventList.count === 0
            anchors.fill: parent
        }
    }
}
