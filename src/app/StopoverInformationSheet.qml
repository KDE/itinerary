/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

/** Details about a stopover, including:
 *  - notes/service alerts
 *  - occupancy
 *  - vehicle ammentities
 *  - operator/provider
 */
SheetDrawer {
    id: root
    /** The KPublicTransport.Stopover to show here. */
    property KPublicTransport.stopover stopover
    /** Notes/service alerts to show.
     *  This defaults to stopover.notes, overriding
     *  makes sense when e.g. showing a journey section departure.
     */
    property list<string> notes: stopover.notes

    /** @c true if there is any content that can be shown here. */
    readonly property bool hasContent: root.notes.length > 0
        || root.stopover.vehicleLayout.combinedFeatures.length > 0
        || root.stopover.aggregatedOccupancy.length > 0
        || root.stopover.route.line.operatorName !== ""

    contentItem: QQC2.ScrollView {
        id: infoPage
        contentWidth: width - effectiveScrollBarWidth
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ColumnLayout {
            width: infoPage.width - infoPage.effectiveScrollBarWidth
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                text: i18n("Information")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.notes.length > 0
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: root.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                font.italic: true
                onLinkActivated: (link) => { Qt.openUrlExternally(link); }
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing
                visible: root.notes.length > 0
            }

            Kirigami.Heading {
                text: i18n("Occupancy")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.stopover.aggregatedOccupancy.length > 0
            }
            Repeater {
                model: root.stopover.aggregatedOccupancy
                delegate: KPublicTransport.OccupancyDelegate {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    required property KPublicTransport.loadInfo modelData
                    occupancyInfo: modelData
                }
            }

            Kirigami.Heading {
                text: i18n("Amenities")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.stopover.vehicleLayout.combinedFeatures.length > 0
            }
            Repeater {
                model: root.stopover.vehicleLayout.combinedFeatures
                delegate: KPublicTransport.FeatureDelegate {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    required property KPublicTransport.feature modelData
                    feature: modelData
                }
            }

            Kirigami.Heading {
                text: i18nc("operator/provider of a rail/bus service", "Operator")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.stopover.route.line.operatorName !== ""
            }
            QQC2.Label {
                text: root.stopover.route.line.operatorName
                visible: text !== ""
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing
            }
        }
    }
}
