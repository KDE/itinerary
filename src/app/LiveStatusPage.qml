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
        positionUpdateInterval: positionAction.checked ? 10 : -1
        journeyUpdateInterval: journeyAction.checked ? 60 : -1
        Component.onCompleted: {
            requestPosition();
            requestJourney();
        }
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

            QtLocation.Map {
                id: map
                anchors.fill: parent
                center: QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                plugin: applicationWindow().osmPlugin()
                zoomLevel: (onboardStatus.hasSpeed && onboardStatus.speed > 600) ? 8 : 12 // zoom out further when flying
                visible: !isNaN(onboardStatus.latitude) && !isNaN(onboardStatus.longitude)
                gesture.acceptedGestures: QtLocation.MapGestureArea.PinchGesture | QtLocation.MapGestureArea.PanGesture
                gesture.preventStealing: true

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
                            Kirigami.Theme.colorSet: Kirigami.Theme.Selection
                            Kirigami.Theme.inherit: false
                            anchors.top: icon.bottom
                            text: Localizer.formatSpeed(onboardStatus.speed)
                            visible: onboardStatus.hasSpeed
                            background: Rectangle { color: Kirigami.Theme.backgroundColor }
                        }
                    }
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.fill: parent
                visible: !map.visible
                text: i18n("Waiting for data...")
            }
        }

        JourneySectionPage {
            journeySection: onboardStatus.journey.sections[0]
            showProgress: true
        }
    }

    footer: Kirigami.NavigationTabBar {
        actions: [
            Kirigami.Action {
                id: positionAction
                text: i18n("Position")
                icon.name: 'map-symbolic'
                onTriggered: swipeView.currentIndex = 0
                checked: swipeView.currentIndex === 0
                enabled: onboardStatus.hasPosition
            },
            Kirigami.Action {
                id: journeyAction
                text: i18n("Journey")
                icon.name: 'view-calendar-day'
                onTriggered: swipeView.currentIndex = 1;
                checked: swipeView.currentIndex === 1
                enabled: onboardStatus.hasJourney
            }
        ]
    }
}
