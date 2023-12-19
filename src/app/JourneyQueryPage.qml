/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels
import org.kde.kpublictransport
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: root

    /** The journey selected by the user on this page. */
    property var journey
    /** The journey to query for. */
    property alias journeyRequest: journeyModel.request
    property alias publicTransportManager: journeyModel.manager

    actions: [
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

    ListView {
        id: journeyView

        clip: true

        delegate: FormCard.FormCard {
            id: top

            width: ListView.view.width

            required property int index
            required property var journey

            JourneyDelegateHeader {
                journey: top.journey
            }

            Repeater {
                id: journeyRepeater
                delegate: JourneySectionDelegate {
                    Layout.fillWidth: true
                    modelLength: journeyRepeater.count - 1
                }
                model: journeyView.currentIndex === top.index ? top.journey.sections : 0
            }

            JourneySummaryDelegate {
                id: summaryButton

                journey: top.journey
                visible: journeyView.currentIndex !== top.index
                onClicked: journeyView.currentIndex = top.index

                Layout.fillWidth: true
            }

            FormCard.FormDelegateSeparator {
                visible: journeyView.currentIndex === top.index
                above: selectButton
            }

            FormCard.FormButtonDelegate {
                id: selectButton

                text: i18n("Select")
                icon.name: "checkmark"
                visible: journeyView.currentIndex === top.index
                enabled: top.journey.disruptionEffect !== Disruption.NoService
                onClicked: root.journey = journey
            }
        }

        model: KSortFilterProxyModel {
            id: sortedJourneyModel

            sourceModel: JourneyQueryModel {
                id: journeyModel
            }
            sortRole: JourneyQueryModel.ScheduledDepartureTime
            dynamicSortFilter: true
            Component.onCompleted: Util.sortModel(sortedJourneyModel, 0, Qt.Ascending)
        }

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

            FormCard.FormCard {
                visible: journeyModel.attributions.length > 0

                FormCard.FormTextDelegate {
                    text: i18n("Data providers:")
                    description: PublicTransport.attributionSummary(journeyModel.attributions)
                    onLinkActivated: Qt.openUrlExternally(link)
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
