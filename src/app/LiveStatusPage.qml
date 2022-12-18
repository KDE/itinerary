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

    OnboardStatus {
        id: onboardStatus
        positionUpdateInterval: 10
        journeyUpdateInterval: 60
    }

    QQC2.SwipeView {
        id: swipeView
        anchors.fill: parent

        Kirigami.Page {
            header: Kirigami.FormLayout {
                QQC2.Label {
                    text: Localizer.formatSpeed(onboardStatus.speed)
                    visible: onboardStatus.hasSpeed
                    Kirigami.FormData.label: i18n("Speed")
                }
            }

            Kirigami.ColumnView.preventStealing: true

            QtLocation.Plugin {
                id: mapPlugin
                required.mapping: QtLocation.Plugin.AnyMappingFeatures
                preferred: ["osm"]
            }

            QtLocation.Map {
                id: map
                anchors.fill: parent
                center: QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                plugin: mapPlugin
                zoomLevel: 12
                gesture.acceptedGestures: QtLocation.MapGestureArea.PinchGesture | QtLocation.MapGestureArea.PanGesture
                gesture.preventStealing: true

                QtLocation.MapQuickItem {
                    coordinate: QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                    anchorPoint {
                        x: icon.width / 2
                        y: onboardStatus.hasHeading ? icon.height / 2 : icon.height
                    }
                    visible: onboardStatus.hasPosition
                    sourceItem: Kirigami.Icon {
                        id: icon
                        source: onboardStatus.hasHeading ? "go-up-symbolic" : "map-symbolic"
                        width: height
                        height: Kirigami.Units.iconSizes.large
                        color: Kirigami.Theme.highlightColor
                        rotation: onboardStatus.hasHeading ? onboardStatus.heading : 0.0
                        transformOrigin: Item.Center
                        onTransformOriginChanged: icon.transformOrigin = Item.Center
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
            },
            Kirigami.Action {
                text: i18n("Journey")
                icon.name: 'view-calendar-day'
                enabled: onboardStatus.hasJourney
                onTriggered: swipeView.currentIndex = 1;
                checked: swipeView.currentIndex === 1
            }
        ]
    }
}
