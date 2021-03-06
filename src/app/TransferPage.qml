/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    property var transfer
    title: i18n("Select Transfer")

    JourneyQueryModel {
        id: journeyModel
        manager: LiveDataManager.publicTransportManager
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
            },
            Kirigami.Action {
                text: i18n("Earlier")
                iconName: "go-up-symbolic"
                onTriggered: journeyModel.queryPrevious()
                enabled: journeyModel.canQueryPrevious
            },
            Kirigami.Action {
                text: i18n("Later")
                iconName: "go-down-symbolic"
                onTriggered: journeyModel.queryNext()
                enabled: journeyModel.canQueryNext
            }
        ]
    }

    function queryJourney() {
        journeyModel.request = TransferManager.journeyRequestForTransfer(transfer);
    }

    Component.onCompleted: {
        if (transfer.floatingLocationType == Transfer.FavoriteLocation) {
            //favLocCombo.currentIndex = favLocCombo.find(transfer.alignment == Transfer.Before ? transfer.fromName : transfer.toName);
        }
        queryJourney();
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
                id: contentLayout
                spacing: Kirigami.Units.smallSpacing

                ListView {
                    delegate: App.JourneySectionDelegate{}
                    model: journeyView.currentIndex == index ? journey.sections : 0
                    implicitHeight: contentHeight
                    width: contentLayout.width
                    boundsBehavior: Flickable.StopAtBounds
                }
                App.JourneySummaryDelegate {
                    journey: top.journey
                    visible: journeyView.currentIndex != index
                    width: parent.width
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

    header: ColumnLayout {
        id: topLayout

        QQC2.Label {
            text: i18n("Preceding arrival %1 at %2", Localizer.formatTime(transfer, "anchorTime"), transfer.fromName);
            visible: transfer.alignment == Transfer.After
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Following departure %1 from %2", Localizer.formatTime(transfer, "anchorTime"), transfer.toName);
            visible: transfer.alignment == Transfer.Before
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
        }

        QQC2.ComboBox {
            id: favLocCombo
            model: FavoriteLocationModel
            visible: transfer.floatingLocationType == Transfer.FavoriteLocation
            textRole: "display"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
            onActivated: {
                var favLoc = delegateModel.items.get(currentIndex)
                console.log(favLoc.model.favoriteLocation);
                root.transfer = TransferManager.setFavoriteLocationForTransfer(root.transfer, favLoc.model.favoriteLocation);
                queryJourney();
            }
        }
    }

    Kirigami.CardsListView {
        id: journeyView
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
