/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    property QtObject controller;

    id: root
    title: i18n("Alternative Connections")

    JourneyQueryModel {
        id: journeyModel
        manager: _liveDataManager.publicTransportManager
        request: controller.journeyRequest
    }

    SortFilterProxyModel {
        id: sortedJourneyModel
        sourceModel: journeyModel
        sortRole: JourneyQueryModel.ScheduledDepartureTime
        dynamicSortFilter: true
        Component.onCompleted: Util.sortModel(sortedJourneyModel, 0, Qt.Ascending)
    }

    Component {
        id: journeyDelegate
        Kirigami.Card {
            id: top
            property var journey: model.journey
            header: JourneyDelegateHeader {
                journey: top.journey
            }

            contentItem: ColumnLayout {
                ListView {
                    delegate: App.JourneySectionDelegate{}
                    model: journeyView.currentIndex == index ? top.journey.sections : 0
                    implicitHeight: contentHeight
                    Layout.fillWidth: true
                    boundsBehavior: Flickable.StopAtBounds
                }
                App.JourneySummaryDelegate {
                    journey: top.journey
                    visible: journeyView.currentIndex != index
                    Layout.fillWidth: true
                }
            }

            onClicked: {
                journeyView.currentIndex = index;
            }

            actions: [
                Kirigami.Action {
                    text: i18n("Save")
                    iconName: "document-save"
                    onTriggered: {
                        replaceWarningSheet.journey = journey
                        replaceWarningSheet.sheetOpen = true
                    }
                    visible: journeyView.currentIndex == index
                }
            ]
        }
    }

    Kirigami.OverlaySheet {
        id: replaceWarningSheet
        property var journey

        QQC2.Label {
            text: i18n("Do you really want to replace your existing reservation with the newly selected journey?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Replace")
                icon.name: "edit-save"
                onClicked: {
                    controller.applyJourney(replaceWarningSheet.journey);
                    applicationWindow().pageStack.pop();
                }
            }
        }
    }

    Kirigami.CardsListView {
        id: journeyView
        anchors.fill: parent
        clip: true
        delegate: journeyDelegate
        model: sortedJourneyModel

        header: QQC2.ToolButton {
            icon.name: "go-up-symbolic"
            visible: journeyModel.canQueryPrevious
            onClicked: journeyModel.queryPrevious()
            x: Kirigami.Units.largeSpacing * 2
            width: journeyView.width - Kirigami.Units.largeSpacing * 4
        }

        footer: ColumnLayout {
            x: Kirigami.Units.largeSpacing * 2
            width: journeyView.width - Kirigami.Units.largeSpacing * 4
            QQC2.ToolButton {
                Layout.fillWidth: true
                icon.name: "go-down-symbolic"
                visible: journeyModel.canQueryNext
                onClicked: journeyModel.queryNext()
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("Data providers: %1", PublicTransport.attributionSummary(journeyModel.attributions))
                visible: journeyModel.attributions.length > 0
                wrapMode: Text.Wrap
                font.pointSize: Kirigami.Units.pointSize * 0.8
                font.italic: true
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: journeyModel.loading
        }

        QQC2.Label {
            anchors.centerIn: parent
            width: parent.width
            text: journeyModel.errorMessage
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }
    }
}
