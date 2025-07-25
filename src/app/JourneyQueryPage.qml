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

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    ListView {
        id: journeyView

        clip: true

        delegate: FormCard.FormCard {
            id: top

            width: ListView.view.width

            required property int index
            required property var journey

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

        section {
            property: "scheduledDepartureDate"
            delegate: TimelineSectionDelegate {
                day: section
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

        Kirigami.LoadingPlaceholder {
            text: if (journeyModel.loading) {
                return i18nc("@info:placeholder", "Loading");
            } else if (journeyModel.errorMessage.length > 0) {
                return i18nc("@info:status", "Error")
            } else {
                return i18nc("@info:placeholder", "No journeys found")
            }
            visible: journeyView.count === 0

            icon.name: journeyModel.errorMessage.length > 0 ? "network-disconnect-symbolic" : ""
            explanation: journeyModel.errorMessage
            progressBar.visible: journeyModel.loading
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
        }
    }

    footer: ColumnLayout {
        spacing: 0
        height: indicator.running ? layout.implicitHeight : 0
        visible: journeyModel.loading && journeyView.count !== 0

        Behavior on height {
            NumberAnimation { duration: Kirigami.Units.shortDuration }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        RowLayout {
            id: layout

            QQC2.BusyIndicator {
                id: indicator
                running: journeyModel.loading && journeyView.count !== 0
            }

            QQC2.Label {
                text: i18n("Still fetching results…")
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
