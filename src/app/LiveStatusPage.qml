/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtLocation 5.11 as QtLocation
import QtPositioning 5.11
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.kpublictransport.onboard 1.0
import org.kde.itinerary 1.0

Kirigami.Page {
    title: i18n("Live Status")
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    OnboardStatus {
        id: onboardStatus
        positionUpdateInterval: 10
        journeyUpdateInterval: 60
    }

    QQC2.SwipeView {
        id: swipeView
        anchors.fill: parent

        Kirigami.Page {
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            Kirigami.ColumnView.preventStealing: true

            QtLocation.Plugin {
                id: mapPlugin
                name: "osm"
                QtLocation.PluginParameter { name: "osm.useragent"; value: ApplicationController.userAgent }
                QtLocation.PluginParameter { name: "osm.mapping.providersrepository.address"; value: "https://autoconfig.kde.org/qtlocation/" }
            }

            QtLocation.Map {
                id: map
                anchors.fill: parent
                center: QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                plugin: mapPlugin
                zoomLevel: 12

                QtLocation.MapQuickItem {
                    coordinate: QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                    anchorPoint {
                        x: icon.width / 2
                        y: onboardStatus.hasHeading ? icon.height / 2 : icon.height
                    }
                    visible: onboardStatus.hasPosition
                    sourceItem: Item {
                        Kirigami.Icon {
                            id: icon
                            source: onboardStatus.hasHeading ? "go-up-symbolic" : "map-symbolic"
                            width: height
                            height: Kirigami.Units.iconSizes.medium
                            color: Kirigami.Theme.highlightColor
                            rotation: onboardStatus.hasHeading ? onboardStatus.heading : 0.0
                            transformOrigin: Item.Center
                            onTransformOriginChanged: icon.transformOrigin = Item.Center
                        }
                        QQC2.Label {
                            anchors.top: icon.bottom
                            color: Kirigami.Theme.activeTextColor
                            text: Localizer.formatSpeed(onboardStatus.speed)
                            visible: Localizer.hasSpeed
                            background: Rectangle { color: Kirigami.Theme.activeBackgroundColor }
                        }
                    }
                }
            }
        }

        JourneySectionPage {
            journeySection: onboardStatus.journey.sections[0]
        }
    }

    footer: Kirigami.NavigationTabBar {
        actions: [
            Kirigami.Action {
                text: i18n("Position")
                icon.name: 'map-symbolic'
                onTriggered: swipeView.currentIndex = 0
                checked: swipeView.currentIndex === 0
                enabled: onboardStatus.hasPosition
            },
            Kirigami.Action {
                text: i18n("Journey")
                icon.name: 'view-calendar-day'
                onTriggered: swipeView.currentIndex = 1;
                checked: swipeView.currentIndex === 1
                enabled: onboardStatus.hasJourney
            }
        ]
    }
}
