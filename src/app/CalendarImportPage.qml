/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import internal.org.kde.kcalendarcore as KCalendarCore
import org.kde.itinerary
import "." as App

Kirigami.ScrollablePage {
    id: root

    property alias calendar: importModel.calendar

    title: i18n("Calendar Import")

    CalendarImportModel {
        id: importModel
    }

    ListView {
        id: eventList
        model: importModel

        delegate: QQC2.ItemDelegate {
            highlighted: model.selected
            width: ListView.view.width
            contentItem: GridLayout {
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
            text: i18n("No importable events found in this calendar.")
            visible: eventList.count === 0
            anchors.fill: parent
        }

        FloatingButton {
            anchors {
                right: parent.right
                rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
                bottom: parent.bottom
                bottomMargin: Kirigami.Units.largeSpacing
            }
            action: Kirigami.Action {
                icon.name: "document-open"
                text: i18n("Import selected events")
                enabled: importModel.hasSelection
                onTriggered: {
                    importModel.selectedReservations().forEach(r => ReservationManager.importReservation(r));
                    applicationWindow().pageStack.pop();
                }
            }
    }
    }
}
