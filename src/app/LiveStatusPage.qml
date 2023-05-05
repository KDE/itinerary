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
            map.autoPositionMap();
        }

        onPositionChanged: map.autoPositionMap();
    }

    QQC2.SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: !onboardStatus.supportsPosition ? 1 : 0

        Kirigami.Page {
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            Kirigami.ColumnView.preventStealing: true

            QtLocation.Map {
                id: map
                property bool autoFollow: true

                function autoPositionMap() {
                    if (map.autoFollow && !isNaN(onboardStatus.latitude) && !isNaN(onboardStatus.longitude)) {
                        map.center = QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                        map.zoomLevel = (onboardStatus.hasSpeed && onboardStatus.speed > 600) ? 8 : 12 // zoom out further when flying
                        map.autoFollow = true;
                    }
                }

                anchors.fill: parent
                plugin: applicationWindow().osmPlugin()
                visible: !isNaN(onboardStatus.latitude) && !isNaN(onboardStatus.longitude)
                gesture.acceptedGestures: QtLocation.MapGestureArea.PinchGesture | QtLocation.MapGestureArea.PanGesture
                gesture.preventStealing: true
                onCopyrightLinkActivated: Qt.openUrlExternally(link)
                onZoomLevelChanged: autoFollow = false
                onCenterChanged: autoFollow = false

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

            QQC2.Button {
                checkable: true
                checked: map.autoFollow
                icon.name: "map-symbolic"
                text: i18n("Automatically follow on the map")
                display: QQC2.Button.IconOnly
                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
                onCheckedChanged: {
                    map.autoFollow = checked;
                    map.autoPositionMap();
                }
                anchors {
                    top: map.top
                    right: map.right
                    margins: Kirigami.Units.largeSpacing
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

            Kirigami.PlaceholderMessage {
                anchors.fill: parent
                visible: !onboardStatus.hasJourney
                text: i18n("Waiting for data...")
            }
        }
    }

    footer: Kirigami.NavigationTabBar {
        visible: onboardStatus.supportsPosition && onboardStatus.supportsJourney

        actions: [
            Kirigami.Action {
                id: positionAction
                text: i18n("Position")
                icon.name: 'map-symbolic'
                onTriggered: swipeView.currentIndex = 0
                checked: swipeView.currentIndex === 0
                enabled: onboardStatus.hasPosition
                visible: onboardStatus.supportsPosition
            },
            Kirigami.Action {
                id: journeyAction
                text: i18n("Journey")
                icon.name: 'view-calendar-day'
                onTriggered: swipeView.currentIndex = 1;
                checked: swipeView.currentIndex === 1
                enabled: onboardStatus.hasJourney
                visible: onboardStatus.supportsJourney
            }
        ]
    }
}
