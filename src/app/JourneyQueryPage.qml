/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

Kirigami.ScrollablePage {
    id: root

    /** The journey selected by the user on this page. */
    property var journey
    /** The journey to query for. */
    property alias journeyRequest: journeyModel.request
    property alias publicTransportManager: journeyModel.manager

    contextualActions: [
        Kirigami.Action {
            text: i18n("Earlier")
            icon.name: "go-up-symbolic"
            onTriggered: journeyModel.queryPrevious()
            enabled: journeyModel.canQueryPrevious
        },
        Kirigami.Action {
            text: i18n("Later")
            icon.name: "go-down-symbolic"
            onTriggered: journeyModel.queryNext()
            enabled: journeyModel.canQueryNext
        }
    ]

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    leftPadding: 0
    rightPadding: 0

    JourneyQueryModel {
        id: journeyModel
    }

    KSortFilterProxyModel {
        id: sortedJourneyModel
        sourceModel: journeyModel
        sortRole: JourneyQueryModel.ScheduledDepartureTime
        dynamicSortFilter: true
        Component.onCompleted: Util.sortModel(sortedJourneyModel, 0, Qt.Ascending)
    }

    Component {
        id: journeyDelegate

        MobileForm.FormCard {
            id: top

            width: ListView.view.width

            required property int index
            required property var journey

            contentItem: ColumnLayout {
                id: contentLayout
                spacing: 0

                JourneyDelegateHeader {
                    journey: top.journey
                }

                Repeater {
                    id: journeyRepeater
                    delegate: App.JourneySectionDelegate {
                        Layout.fillWidth: true
                        modelLength: journeyRepeater.count - 1
                    }
                    model: journeyView.currentIndex === top.index ? top.journey.sections : 0
                }

                App.JourneySummaryDelegate {
                    id: summaryButton

                    journey: top.journey
                    visible: journeyView.currentIndex !== top.index
                    onClicked: journeyView.currentIndex = top.index

                    Layout.fillWidth: true
                }

                MobileForm.FormDelegateSeparator {
                    visible: journeyView.currentIndex === top.index
                    above: selectButton
                }

                MobileForm.FormButtonDelegate {
                    id: selectButton

                    text: i18n("Select")
                    icon.name: "checkmark"
                    visible: journeyView.currentIndex === top.index
                    enabled: top.journey.disruptionEffect !== Disruption.NoService
                    onClicked: root.journey = journey
                }
            }
        }
    }

    ListView {
        id: journeyView

        clip: true
        delegate: journeyDelegate
        model: sortedJourneyModel
        spacing: Kirigami.Units.largeSpacing

        header: VerticalNavigationButton {
            visible: journeyModel.canQueryPrevious
            width: journeyView.width
            text: i18nc("@action:button", "Load earlier connections")
            iconName: "go-up-symbolic"
            onClicked: journeyModel.queryPrevious()
        }

        footer: VerticalNavigationButton {
            visible: journeyModel.canQueryNext
            width: journeyView.width
            iconName: "go-down-symbolic"
            text: i18nc("@action:button", "Load later connections")
            onClicked: journeyModel.queryNext()

            MobileForm.FormCard {
                visible: journeyModel.attributions.length > 0

                Layout.fillWidth: true

                contentItem: ColumnLayout {
                    spacing: 0

                    MobileForm.FormTextDelegate {
                        text: i18n("Data providers:")
                        description: PublicTransport.attributionSummary(journeyModel.attributions)
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                }
            }
        }

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: journeyModel.loading
        }

        QQC2.Label {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
            text: journeyModel.errorMessage
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }
    }
}
