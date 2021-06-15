/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    property QtObject controller;
    property alias publicTransportManager: journeyModel.manager

    id: root
    title: i18n("Alternative Connections")
    contextualActions: [
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


    JourneyQueryModel {
        id: journeyModel
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

            contentItem: Column {
                id: contentLayout
                spacing: Kirigami.Units.smallSpacing

                ListView {
                    delegate: App.JourneySectionDelegate{}
                    model: journeyView.currentIndex == index ? top.journey.sections : 0
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
                    text: i18n("Save")
                    icon.name: "document-save";
                    visible: journeyView.currentIndex == index
                    onClicked: {
                        replaceWarningSheet.journey = journey
                        replaceWarningSheet.sheetOpen = true
                    }
                }
            }

            onClicked: {
                journeyView.currentIndex = index;
            }
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

        footer: Column {
            id: footerLayout
            spacing: Kirigami.Units.smallSpacing

            x: Kirigami.Units.largeSpacing * 2
            width: journeyView.width - Kirigami.Units.largeSpacing * 4
            QQC2.ToolButton {
                width: footerLayout.width
                icon.name: "go-down-symbolic"
                visible: journeyModel.canQueryNext
                onClicked: journeyModel.queryNext()
            }
            QQC2.Label {
                width: footerLayout.width
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
