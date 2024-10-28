/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
            }

            QQC2.Label {
                id: departureTime
                Layout.rightMargin: delayLabel.visible ? 0:Kirigami.Units.largeSpacing
                text: Localizer.formatTime(stopInfoDrawer.stop, "scheduledDepartureTime")
                font.strikeout: stopInfoDrawer.stop.disruptionEffect === KPublicTransport.Disruption.NoService
            }
            QQC2.Label {
                id: delayLabel
                Layout.rightMargin: Kirigami.Units.largeSpacing
                text: (stopInfoDrawer.stop.departureDelay >= 0 ? "+" : "") + stopInfoDrawer.stop.departureDelay
                color: stopInfoDrawer.stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: departureTime.visible && stopInfoDrawer.stop.hasExpectedDepartureTime && stopInfoDrawer.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
            }
        }
    }

    contentItem: Component{
        ColumnLayout {
            id: contentLayout
            spacing:0
            width: stopInfoDrawer.width
            FormCard.FormTextDelegate {
                Layout.fillWidth: true
                id: platformDelegate
                text: i18n("Platform:")
                description: stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                visible: description
            }
            FormCard.AbstractFormDelegate {
                Layout.fillWidth: true
                visible: PublicTransport.maximumOccupancy(stopInfoDrawer.stop.loadInformation) != KPublicTransport.Load.Unknown
                    contentItem: ColumnLayout {
                        spacing: Kirigami.Units.mediumSpacing

                        QQC2.Label {
                            text: i18n("Occupancy:")
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            Accessible.ignored: true
                        }
                        KPublicTransport.OccupancyIndicator {
                            occupancy: PublicTransport.maximumOccupancy(stopInfoDrawer.stop.loadInformation)
                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        }
                    }

                background: Item{}
            }
            Item{Layout.fillHeight: true}
            FormCard.FormDelegateSeparator {}
            FormCard.FormButtonDelegate {
                Layout.fillWidth: true
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
