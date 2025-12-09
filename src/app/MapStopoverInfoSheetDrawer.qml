/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

SheetDrawer {
    id: stopInfoDrawer
    property KPublicTransport.stopover stop
    property bool isDeparture: false
    property bool isArrival: false

    headerItem: Component {
        RowLayout {
            id: headerLayout
            Kirigami.Heading {
                text: stopInfoDrawer.stop.stopPoint.name
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                elide: Qt.ElideRight
                font.strikeout: stopInfoDrawer.stop.disruptionEffect === KPublicTransport.Disruption.NoService
            }

            readonly property bool showDepartureTime: (stopInfoDrawer.isDeparture && !isNaN(stopInfoDrawer.stop.scheduledDepartureTime.getTime())) || !stopInfoDrawer.stop.scheduledArrivalTime

            QQC2.Label {
                id: departureTime
                Layout.rightMargin: delayLabel.visible ? 0 : Kirigami.Units.largeSpacing
                text: Localizer.formatTime(stopInfoDrawer.stop, headerLayout.showDepartureTime ? "scheduledDepartureTime" : "scheduledArrivalTime")
                font.strikeout: stopInfoDrawer.stop.disruptionEffect === KPublicTransport.Disruption.NoService
            }
            QQC2.Label {
                id: delayLabel
                readonly property int delay: headerLayout.showDepartureTime ? stopInfoDrawer.stop.departureDelay : stopInfoDrawer.stop.arrivalDelay

                Layout.rightMargin: Kirigami.Units.largeSpacing
                text: (delayLabel.delay >= 0 ? "+" : "") + delayLabel.delay
                color: delayLabel.delay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: headerLayout.showDepartureTime ? stopInfoDrawer.stop.hasExpectedDepartureTime : stopInfoDrawer.stop.hasExpectedArrivalTime && stopInfoDrawer.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
            }
        }
    }

    contentItem: QQC2.ScrollView {
        id: scrollView
        contentWidth: width - effectiveScrollBarWidth
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ColumnLayout {
            id: contentLayout

            spacing: 0
            width: scrollView.width - scrollView.effectiveScrollBarWidth

            FormCard.FormTextDelegate {
                id: platformDelegate

                text: i18n("Platform:")
                description: stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                visible: description
            }

            FormCard.FormDelegateSeparator { visible: platformDelegate.visible }

            FormCard.AbstractFormDelegate {
                id: occupancyDelegate

                visible: stopInfoDrawer.stop.aggregatedOccupancy.length > 0
                background: null
                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.mediumSpacing

                    QQC2.Label {
                        text: i18n("Occupancy:")
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Accessible.ignored: true
                    }
                    Repeater {
                        model: stopInfoDrawer.stop.aggregatedOccupancy
                        delegate: KPublicTransport.OccupancyDelegate {
                            required property KPublicTransport.loadInfo modelData
                            occupancyInfo: modelData
                        }
                    }
                }
            }

            FormCard.FormDelegateSeparator { visible: occupancyDelegate.visible }

            FormCard.FormTextDelegate {
                id: informationDelegate

                text: i18n("Information:")
                description: stopInfoDrawer.stop.notes.join("<br/>")
                descriptionItem.textFormat: Text.RichText
                descriptionItem.wrapMode: Text.Wrap
                descriptionItem.font.italic: true
                visible: stopInfoDrawer.stop.notes.length > 0
                onLinkActivated: (link) => { Qt.openUrlExternally(link); }
            }

            Item {
                Layout.fillHeight: true
            }

            FormCard.FormDelegateSeparator { visible: informationDelegate.visible }

            FormCard.FormButtonDelegate {
                text: i18n("Show indoor map")
                icon.name: "map-symbolic"
                onClicked: {
                    stopInfoDrawer.close();
                    const args = {
                        coordinate: Qt.point(stopInfoDrawer.stop.stopPoint.longitude, stopInfoDrawer.stop.stopPoint.latitude),
                        placeName: stopInfoDrawer.stop.stopPoint.name
                    };
                    if (!stopInfoDrawer.isDeparture) {
                        args.arrivalPlatformName = stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                        args.arrivalPlatformMode = PublicTransport.lineModeToPlatformMode(stopInfoDrawer.stop.route.line.mode);
                        args.arrivalPlatformIfopt = stopInfoDrawer.stop.stopPoint.identifier("ifopt");
                    }
                    if (!stopInfoDrawer.isArrival) {
                        args.departurePlatformName = stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                        args.departurePlatformMode = PublicTransport.lineModeToPlatformMode(stopInfoDrawer.stop.route.line.mode);
                        args.departurePlatformIfopt = stopInfoDrawer.stop.stopPoint.identifier("ifopt");
                    }

                    // ensure the map page ends up on top
                    if (applicationWindow().pageStack.layers.depth < 2)
                        applicationWindow().pageStack.push(indoorMapPage, args);
                    else
                        applicationWindow().pageStack.layers.push(indoorMapPage, args);
                }
            }
        }
    }
}
