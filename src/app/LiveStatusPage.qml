/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Effects as Effects
import QtLocation as QtLocation
import QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.kpublictransport.onboard
import org.kde.itinerary

Kirigami.Page {
    id: root

    title: i18n("Live Status")

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property string mapStyle: Settings.read("LiveStatusPage/MapStyle", "");
    property QtLocation.Map overlayMap

    // this whole convoluted setup is needed due to Map.plugin being a write-once property
    // so we can neither just swap that at runtime nor can we have property bindings on plugin
    // parameters
    onMapStyleChanged: {
        if (root.overlayMap)
            root.overlayMap.destroy()
        // must not be map as parent, otherwise the layer effect applies to us
        root.overlayMap = overlayMapComponent.createObject(map.parent);
        root.overlayMap.plugin = root.mapStyle === "" ? overlayPlugin : railwayMapPlugin.createObject(root.overlayMap);
        root.overlayMap.activeMapType = root.overlayMap.supportedMapTypes[root.overlayMap.supportedMapTypes.length - 1]
        Settings.write("LiveStatusPage/MapStyle", root.mapStyle);
    }
    Component.onCompleted: {
        if (!root.overlayMap)
            root.onMapStyleChanged()
    }

    QQC2.ActionGroup { id: mapStyleGroup }
    actions: [
        Kirigami.Action {
            text: matrixBeacon.isActive ? i18n("Stop Location Sharing") : i18n("Share Location via Matrix")
            icon.name: matrixBeacon.isActive ? "dialog-cancel" : "emblem-shared-symbolic"
            onTriggered: matrixBeacon.isActive ? matrixBeacon.stop() : matrixRoomSheet.open()
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === ""
            text: i18nc("map style", "Normal map")
            icon.name: "map-gnomonic"
            // TODO only for rail-bound modes
            visible: onboardStatus.supportsPosition
            onTriggered: root.mapStyle = ""
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "standard"
            text: i18nc("map style", "Railway infrastructure map")
            icon.name: "map-gnomonic"
            visible: onboardStatus.supportsPosition
            onTriggered: root.mapStyle = "standard"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "signals"
            text: i18nc("map style", "Railway signalling map")
            icon.name: "map-gnomonic"
            visible: onboardStatus.supportsPosition
            onTriggered: root.mapStyle = "signals"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "maxspeed"
            text: i18nc("map style", "Railway speed map")
            icon.name: "map-gnomonic"
            visible: onboardStatus.supportsPosition
            onTriggered: root.mapStyle = "maxspeed"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "electrification"
            text: i18nc("map style", "Railway electrification map")
            icon.name: "map-gnomonic"
            visible: onboardStatus.supportsPosition
            onTriggered: root.mapStyle = "electrification"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "gauge"
            text: i18nc("map style", "Railway gauge map")
            icon.name: "map-gnomonic"
            visible: onboardStatus.supportsPosition
            onTriggered: root.mapStyle = "gauge"
        }
    ]

    OnboardStatus {
        id: onboardStatus
        positionUpdateInterval: positionAction.checked ? 10 : -1
        journeyUpdateInterval: journeyAction.checked ? 60 : -1
        Component.onCompleted: {
            requestPosition();
            requestJourney();
            map.autoPositionMap();
        }

        onPositionChanged: {
            map.autoPositionMap();
            matrixBeacon.updateLocation(onboardStatus.latitude, onboardStatus.longitude, onboardStatus.heading, onboardStatus.speed, onboardStatus.altitude);
        }
    }

    MatrixRoomSelectionSheet {
        id: matrixRoomSheet
        onRoomSelected: {
            shareConfirmDialog.room = room;
            shareConfirmDialog.open();
        }
    }

    MatrixBeacon {
        id: matrixBeacon
        connection: MatrixController.manager.connection
    }

    Kirigami.PromptDialog {
        id: shareConfirmDialog

        property var room

        title: i18n("Share Live Location")
        subtitle: room ? i18n("Do you really want to share your current location to the Matrix channel %1?", room.displayName) : ""

        standardButtons: QQC2.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Share")
                icon.name: "emblem-shared-symbolic"
                visible: MatrixController.isAvailable
                enabled: MatrixController.manager.connected
                onTriggered: {
                    console.log(shareConfirmDialog.room.id);
                    matrixBeacon.roomId = shareConfirmDialog.room.id;
                    if (onboardStatus.hasJourney && onboardStatus.journey.sections[0].route.line.name) {
                        const jny = onboardStatus.journey.sections[0];
                        const name = jny.route.line.modeString + " " + jny.route.line.name;
                        matrixBeacon.start(name.trim());
                    } else {
                        matrixBeacon.start("Live Position");
                    }
                    shareConfirmDialog.close();
                }
            }
        ]
        closePolicy: QQC2.Popup.CloseOnEscape
    }

    QQC2.SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: !onboardStatus.supportsPosition ? 1 : 0
        interactive: footerTabBar.visible

        Kirigami.Page {
            id: mapPage
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            Kirigami.ColumnView.preventStealing: true

            MapView {
                id: map
                copyrightsVisible: root.mapStyle === ""
                property bool autoFollow: true

                function autoPositionMap() {
                    if (map.autoFollow && !isNaN(onboardStatus.latitude) && !isNaN(onboardStatus.longitude)) {
                        map.center = QtPositioning.coordinate(onboardStatus.latitude, onboardStatus.longitude)
                        map.zoomLevel = (onboardStatus.hasSpeed && onboardStatus.speed > 600) ? 8 : 12 // zoom out further when flying
                        map.autoFollow = true;
                    }
                }

                anchors.fill: parent
                visible: !isNaN(onboardStatus.latitude) && !isNaN(onboardStatus.longitude)
                onZoomLevelChanged: autoFollow = false
                onCenterChanged: autoFollow = false

                layer.enabled: root.mapStyle !== ""
                layer.effect: Effects.MultiEffect {
                    saturation: -1.0
                }
            }

            QtLocation.Plugin {
                id: overlayPlugin
                name: "itemsoverlay"
            }

            Component {
                id: railwayMapPlugin
                QtLocation.Plugin {
                    name: "osm"
                    QtLocation.PluginParameter { name: "osm.useragent"; value: ApplicationController.userAgent }
                    QtLocation.PluginParameter { name: "osm.mapping.custom.host"; value: "https://tiles.openrailwaymap.org/" + root.mapStyle + "/%z/%x/%y.png" }
                    QtLocation.PluginParameter { name: "osm.mapping.providersrepository.disabled"; value: true }
                    QtLocation.PluginParameter { name: "osm.mapping.custom.datacopyright"; value: "OpenStreetMap contributors" }
                    QtLocation.PluginParameter { name: "osm.mapping.custom.mapcopyright"; value: "OpenRailwayMap" }
                    QtLocation.PluginParameter { name: "osm.mapping.cache.directory"; value: Util.cacheLocation("QtLocation-OpenRailwayMap/" + root.mapStyle) }
                }
            }

            Component {
                id: overlayMapComponent
                QtLocation.Map {
                    id: mapOverlay
                    anchors.fill: map
                    center: map.center
                    color: 'transparent'
                    minimumFieldOfView: map.minimumFieldOfView
                    maximumFieldOfView: map.maximumFieldOfView
                    minimumTilt: map.minimumTilt
                    maximumTilt: map.maximumTilt
                    minimumZoomLevel: map.minimumZoomLevel
                    maximumZoomLevel: map.maximumZoomLevel
                    zoomLevel: map.zoomLevel
                    tilt: map.tilt;
                    bearing: map.bearing
                    fieldOfView: map.fieldOfView

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
                                isMask: true
                                rotation: onboardStatus.hasHeading ? onboardStatus.heading : 0.0
                                transformOrigin: Item.Center
                                onTransformOriginChanged: icon.transformOrigin = Item.Center
                            }
                            QQC2.Label {
                                Kirigami.Theme.colorSet: Kirigami.Theme.Selection
                                Kirigami.Theme.inherit: false
                                anchors.top: icon.bottom
                                text: Localizer.formatSpeed(onboardStatus.speed, Settings.distanceFormat)
                                visible: onboardStatus.hasSpeed
                                background: Rectangle { color: Kirigami.Theme.backgroundColor }
                            }
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
                text: i18n("Waiting for data…")
            }
        }

        JourneySectionPage {
            journeySection: onboardStatus.journey.sections[0]
            showProgress: true
            enableMapView: false

            Kirigami.PlaceholderMessage {
                anchors.fill: parent
                visible: !onboardStatus.hasJourney
                text: i18n("Waiting for data…")
            }
        }
    }

    footer: Kirigami.NavigationTabBar {
        id: footerTabBar
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
