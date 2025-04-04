/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

FormCard.AbstractFormDelegate {
    id: root

    required property var journey

    contentItem: ColumnLayout {
        id: mainLayout

        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Kirigami.Heading {
                id: departureTimeLabel

                level: 4
                text: Localizer.formatTime(root.journey, "scheduledDepartureTime")
                font {
                    weight: Font.DemiBold
                    strikeout: root.journey.disruptionEffect === Disruption.NoService
                }
            }

            Kirigami.Heading {
                id: separatorLabel

                level: 4
                text: '-'
            }

            Kirigami.Heading {
                id: arrivalTimeLabel

                level: 4
                text: Localizer.formatTime(root.journey, "scheduledArrivalTime")
                font {
                    weight: Font.DemiBold
                    strikeout: root.journey.disruptionEffect === Disruption.NoService
                }
            }

            QQC2.Label {
                text: ' | ' + Localizer.formatDuration(root.journey.duration) + ' | ' + i18ncp("number of switches to another transport", "One change", "%1 changes", root.journey.numberOfChanges)
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            KPublicTransport.OccupancyIndicator {
                occupancy: root.journey.maximumOccupancy
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.minimumWidth: Kirigami.Units.iconSizes.small
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: (root.journey.hasExpectedDepartureTime || root.journey.hasExpectedArrivalTime) && root.journey.disruptionEffect !== Disruption.NoService

            Layout.fillWidth: true

            Kirigami.Heading {
                readonly property bool hasDepartureDelay: root.journey.departureDelay > 0

                level: 4
                text: root.journey.hasExpectedDepartureTime ? Localizer.formatTime(root.journey, "expectedDepartureTime") : ''
                color: hasDepartureDelay ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                font.weight: Font.DemiBold
                Layout.minimumWidth: departureTimeLabel.width
            }

            Item {
                Layout.minimumWidth: separatorLabel.width
            }

            Kirigami.Heading {
                readonly property bool hasArrivalDelay: root.journey.arrivalDelay > 0

                level: 4
                text: root.journey.hasExpectedArrivalTime ? Localizer.formatTime(root.journey, "expectedArrivalTime") : ''
                color: hasArrivalDelay ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                font.weight: Font.DemiBold
                Layout.minimumWidth: arrivalTimeLabel.width
            }
        }

        KPublicTransport.JourneyHorizontalBar {
            id: sectionsRow
            Layout.fillWidth: true
            journey: root.journey
        }

        QQC2.Label {
            text: i18n("Click to extend")

            Layout.fillWidth: true
        }
    }
}
