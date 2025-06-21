// SPDX-FileCopyrightText: ⓒ 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

/** List view delegate for showing entries in a departure/arrival board.
 *  TODO move to KPublicTransport once KCoreAddon's formatRelativeDateTime's narrow format support is upstream
 */
FormCard.AbstractFormDelegate {
    id: delegateRoot

    /** The KPublicTransport.Stopover to show. */
    required property KPublicTransport.stopover stopover
    /** Whether to show this as an arrival or departure. */
    property bool isArrival: false

    contentItem: GridLayout {
        id: topLayout
        rowSpacing: Kirigami.Units.largeSpacing * 2
        columnSpacing: rowSpacing
        rows: 3
        columns: 2

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 0

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                KPublicTransport.TransportNameControl {
                    line: delegateRoot.stopover.route.line
                }

                Kirigami.Heading {
                    level: 3
                    text: delegateRoot.stopover.route.direction
                    visible: delegateRoot.stopover.route.direction.length > 0
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            Flow {
                spacing: Kirigami.Units.smallSpacing

                Layout.fillWidth: true

                KPublicTransport.ExpectedTimeLabel {
                    id: expectedTimeLabel
                    stopover: delegateRoot.stopover
                    delay: delegateRoot.isArrival ? delegateRoot.stopover.arrivalDelay : delegateRoot.stopover.departureDelay
                    // TODO hide scheduledTime when we are not showing relative times on the right and delay is 0
                    scheduledTime: delay != 0 ? Localizer.formatTime(delegateRoot.stopover, delegateRoot.isArrival ? "scheduledArrivalTime" : "scheduledDepartureTime") : ""
                    hasExpectedTime: delegateRoot.isArrival ? delegateRoot.stopover.hasExpectedArrivalTime : delegateRoot.stopover.hasExpectedDepartureTime
                    visible: hasExpectedTime || delegateRoot.stopover.disruptionEffect === KPublicTransport.Disruption.NoService // TODO show also when we show relative times, see below
                }

                KPublicTransport.OccupancyIndicator {
                    occupancy: delegateRoot.stopover.maximumOccupancy
                    height: Kirigami.Units.iconSizes.small
                    width: Kirigami.Units.iconSizes.small
                }

                Repeater {
                    model: delegateRoot.stopover.features
                    delegate: KPublicTransport.FeatureIcon {
                        required property KPublicTransport.feature modelData
                        feature: modelData
                        height: Kirigami.Units.iconSizes.small
                        width: Kirigami.Units.iconSizes.small
                    }
                }
            }

            // TODO ideally this would not be shown when having a known pre-defined stop point (with the platform info added in the row above)
            // it's crucial when doing vicinity searches though, where the stop points are unknown and can vary
            RowLayout {
                spacing: 0
                Layout.fillWidth: true
                QQC2.Label {
                    id: stopNameLabel
                    text: i18nc("departure stop", "From %1", delegateRoot.stopover.stopPoint.name)
                    visible: delegateRoot.stopover.stopPoint.name !== ""
                    elide: Text.ElideMiddle
                    Layout.maximumWidth: delegateRoot.width - relativeTimeLabel.implicitWidth - platformLabel.implicitWidth - topLayout.columnSpacing - delegateRoot.leftPadding - delegateRoot.rightPadding
                }
                QQC2.Label {
                    id: platformLabel
                    text: (stopNameLabel.visible ? ' · ' : '') + i18nc("@info", "Platform %1", delegateRoot.stopover.hasExpectedPlatform ? delegateRoot.stopover.expectedPlatform : delegateRoot.stopover.scheduledPlatform)
                    color: delegateRoot.stopover.platformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    visible: delegateRoot.stopover.hasExpectedPlatform || delegateRoot.stopover.scheduledPlatform !== ""
                }
            }
        }

        Kirigami.Heading {
            id: relativeTimeLabel
            Layout.row: 0
            Layout.column: 1

            font.strikeout: delegateRoot.stopover.disruptionEffect === KPublicTransport.Disruption.NoService

            // TODO once KFormat::formatRelativeDateTime supports narrow formatting, use that instead when within 1h of now
            text: delegateRoot.isArrival ?
                Localizer.formatTime(delegateRoot.stopover, delegateRoot.stopover.hasExpectedArrivalTime ? "expectedArrivalTime" : "scheduledArrivalTime") :
                Localizer.formatTime(delegateRoot.stopover, delegateRoot.stopover.hasExpectedDepartureTime ? "expectedDepartureTime" : "scheduledDepartureTime")
        }

        QQC2.Label {
            id: notesLabel
            Layout.fillWidth: true
            Layout.row: 1
            Layout.column: 0
            Layout.columnSpan: 2
            Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount

            text: delegateRoot.stopover.notes.join("<br/>")
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: 3
            clip: true
            visible: delegateRoot.stopover.notes.length > 0
            font.italic: true
            onLinkActivated: (link) => { Qt.openUrlExternally(link); }
        }
        Kirigami.LinkButton {
            Layout.row: 2
            Layout.column: 0
            Layout.columnSpan: 2

            text: i18nc("@action:button", "Show More…")
            visible: notesLabel.implicitHeight > notesLabel.height
            onClicked: delegateRoot.clicked()
        }
    }
}
