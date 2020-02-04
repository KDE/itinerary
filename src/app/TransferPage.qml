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
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    property var transfer
    title: i18n("Select Transfer")

    JourneyQueryModel {
        id: journeyModel
        manager: _liveDataManager.publicTransportManager
    }

    SortFilterProxyModel {
        id: sortedJourneyModel
        sourceModel: journeyModel
        sortRole: JourneyQueryModel.ScheduledDepartureTime
        dynamicSortFilter: true
        Component.onCompleted: Util.sortModel(sortedJourneyModel, 0, Qt.Ascending)
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                text: i18n("Depart Now")
                iconName: "clock"
                onTriggered: {
                    var req = journeyModel.request;
                    req.dateTime = new Date();
                    req.dateTimeMode = JourneyRequest.Departure;
                    journeyModel.request = req;
                }
            },
            Kirigami.Action {
                text: i18n("Discard")
                iconName: "edit-delete"
                onTriggered: {
                    TransferManager.discardTransfer(root.transfer);
                    applicationWindow().pageStack.pop();
                }
            }
        ]
    }

    Component.onCompleted: {
        var req = journeyModel.request;
        req.from = transfer.from;
        req.to = transfer.to;
        req.dateTime = transfer.journeyTime;
        req.dateTimeMode = transfer.alignment == Transfer.Before ? JourneyRequest.Arrival : JourneyRequest.Departure;
        journeyModel.request = req;
    }

    Component {
        id: journeyDelegate
        Kirigami.Card {
            id: top
            property var journey: model.journey

            header: JourneyDelegateHeader {
                journey: top.journey
            }

            contentItem: Column {
                spacing: Kirigami.Units.largeSpacing
                ListView {
                    delegate: App.JourneySectionDelegate{}
                    model: journeyView.currentIndex == index ? journey.sections : 0
                    implicitHeight: contentHeight
                    width: parent.width
                    boundsBehavior: Flickable.StopAtBounds
                }
                App.JourneySummaryDelegate {
                    journey: top.journey
                    visible: journeyView.currentIndex != index
                }
                QQC2.Button {
                    text: i18n("Select")
                    icon.name: "document-save"
                    visible: journeyView.currentIndex == index
                    onClicked: {
                        TransferManager.setJourneyForTransfer(root.transfer, top.journey);
                        applicationWindow().pageStack.pop();
                    }
                }
            }

            onClicked: {
                journeyView.currentIndex = index;
            }
        }
    }

    /*FavoriteLocationModel {
        id: favLocModel
    }*/

    ColumnLayout {
        id: topLayout
        anchors { top: parent.top; left: parent.left; right: parent.right }

        QQC2.Label {
            text: i18n("Preceding arrival %1 at %2", Localizer.formatTime(transfer, "anchorTime"), transfer.fromName);
            visible: transfer.alignment == Transfer.After
            Layout.fillWidth: true
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Following departure %1 from %2", Localizer.formatTime(transfer, "anchorTime"), transfer.toName);
            visible: transfer.alignment == Transfer.Before
        }

        /*QQC2.ComboBox {
            model: favLocModel
            visible: transfer.floatingLocationType == Transfer.FavoriteLocation
            textRole: "display"
            Layout.fillWidth: true
            // TODO update transfer and re-run journey query
        }*/
    }

    Kirigami.CardsListView {
        id: journeyView
        anchors {
            top: topLayout.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: -2*Kirigami.Units.largeSpacing
            rightMargin: -2*Kirigami.Units.largeSpacing
        }
        clip: true
        delegate: journeyDelegate
        model: sortedJourneyModel

        header: QQC2.ToolButton {
            icon.name: "go-up-symbolic"
            visible: journeyModel.canQueryPrevious
            onClicked: journeyModel.queryPrevious()
            width: journeyView.width - Kirigami.Units.largeSpacing * 4
        }

        footer: ColumnLayout {
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
