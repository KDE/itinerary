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

    readonly property int sectionWithMaxLoad: {
        var loadMax = Load.Unknown;
        var loadMaxIdx = -1;
        for (var i = 0; root.journey != undefined && i < root.journey.sections.length; ++i) {
            var l = PublicTransport.maximumOccupancy(root.journey.sections[i].loadInformation);
            if (l > loadMax) {
                loadMax = l;
                loadMaxIdx = i;
            }
        }
        return loadMaxIdx;
    }

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
                text: ' | ' + Localizer.formatDurationCustom(root.journey.duration) + ' | ' + i18ncp("number of switches to another transport", "One change", "%1 changes", root.journey.numberOfChanges)
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            KPublicTransport.OccupancyIndicator {
                occupancy: sectionWithMaxLoad < 0 ? Load.Unknown : PublicTransport.maximumOccupancy(root.journey.sections[sectionWithMaxLoad].loadInformation)
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
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

        RowLayout {
            id: sectionsRow

            spacing: Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Repeater {
                model: root.journey.sections
                delegate: TransportNameControl {
                    id: sectionDelegate

                    required property var modelData

                    journeySection: modelData

                    Layout.fillWidth: true
                    Layout.maximumWidth: sectionDelegate.modelData.mode === JourneySection.PublicTransport ? Number.POSITIVE_INFINITY : implicitWidth
                }
            }

            // color: PublicTransport.warnAboutSection(modelData) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor

        }

        QQC2.Label {
            text: i18n("Click to extend")

            Layout.fillWidth: true
        }
    }
}
