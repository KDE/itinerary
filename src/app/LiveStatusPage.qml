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
import org.kde.itinerary.matrix

Kirigami.Page {
    id: root

    title: i18n("Live Status")

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    required property OnboardStatus onboardStatus

    property string mapStyle: Settings.read("LiveStatusPage/MapStyle", "");
    readonly property bool isRailBound: !root.onboardStatus.hasJourney || root.onboardStatus.journey.sections[0].route.line.isRailBound
    readonly property string effectiveMapStyle: root.isRailBound ? root.mapStyle : ""

    property QtLocation.Map overlayMap

    // this whole convoluted setup is needed due to Map.plugin being a write-once property
    // so we can neither just swap that at runtime nor can we have property bindings on plugin
    // parameters
    onEffectiveMapStyleChanged: {
        if (root.overlayMap)
            root.overlayMap.destroy()
        // must not be map as parent, otherwise the layer effect applies to us
        root.overlayMap = overlayMapComponent.createObject(map.parent);
        root.overlayMap.plugin = root.effectiveMapStyle === "" ? overlayPlugin : railwayMapPlugin.createObject(root.overlayMap);
        root.overlayMap.activeMapType = root.overlayMap.supportedMapTypes[root.overlayMap.supportedMapTypes.length - 1]
        if (root.isRailBound)
            Settings.write("LiveStatusPage/MapStyle", root.mapStyle);
    }
    Component.onCompleted: {
        if (!root.overlayMap)
            root.onEffectiveMapStyleChanged()
    }

    QQC2.ActionGroup { id: mapStyleGroup }
    actions: [
        Kirigami.Action {
            text: matrixBeacon.isActive ? i18n("Stop Location Sharing") : i18n("Share Location via Matrix")
            icon.name: matrixBeacon.isActive ? "dialog-cancel" : "emblem-shared-symbolic"
            onTriggered: matrixBeacon.isActive ? matrixBeacon.stop() : matrixRoomSheet.open()
            enabled: MatrixController.manager.connected
            visible: MatrixController.isAvailable
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === ""
            text: i18nc("map style", "Normal map")
            icon.name: "map-gnomonic"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            // TODO only for rail-bound modes
            visible: root.onboardStatus.supportsPosition && root.isRailBound
            onTriggered: root.mapStyle = ""
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "standard"
            text: i18nc("map style", "Railway infrastructure map")
            icon.name: "map-gnomonic"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: root.onboardStatus.supportsPosition && root.isRailBound
            onTriggered: root.mapStyle = "standard"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "signals"
            text: i18nc("map style", "Railway signalling map")
            icon.name: "map-gnomonic"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: root.onboardStatus.supportsPosition && root.isRailBound
            onTriggered: root.mapStyle = "signals"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "maxspeed"
            text: i18nc("map style", "Railway speed map")
            icon.name: "map-gnomonic"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: root.onboardStatus.supportsPosition && root.isRailBound
            onTriggered: root.mapStyle = "maxspeed"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "electrification"
            text: i18nc("map style", "Railway electrification map")
            icon.name: "map-gnomonic"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: root.onboardStatus.supportsPosition && root.isRailBound
            onTriggered: root.mapStyle = "electrification"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: mapStyleGroup
            checkable: true
            checked: root.mapStyle === "gauge"
            text: i18nc("map style", "Railway gauge map")
            icon.name: "map-gnomonic"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: root.onboardStatus.supportsPosition && root.isRailBound
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
            matrixBeacon.updateLocation(root.onboardStatus.latitude, root.onboardStatus.longitude, root.onboardStatus.heading, root.onboardStatus.speed, root.onboardStatus.altitude);
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
                    if (root.onboardStatus.hasJourney && root.onboardStatus.journey.sections[0].route.line.name) {
                        const jny = root.onboardStatus.journey.sections[0];
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
        currentIndex: !root.onboardStatus.supportsPosition ? 1 : 0
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
                copyrightsVisible: root.effectiveMapStyle === ""
                property bool autoFollow: true

                function autoPositionMap() {
                    if (map.autoFollow && !isNaN(root.onboardStatus.latitude) && !isNaN(root.onboardStatus.longitude)) {
                        map.center = QtPositioning.coordinate(root.onboardStatus.latitude, root.onboardStatus.longitude)
                        map.zoomLevel = (root.onboardStatus.hasSpeed && root.onboardStatus.speed > 600) ? 8 : 12 // zoom out further when flying
                        map.autoFollow = true;
                    }
                }

                anchors.fill: parent
                visible: !isNaN(root.onboardStatus.latitude) && !isNaN(root.onboardStatus.longitude)
                onZoomLevelChanged: autoFollow = false
                onCenterChanged: autoFollow = false

                layer.enabled: root.effectiveMapStyle !== ""
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
                    z: map.z + 1

                    QtLocation.MapQuickItem {
                        coordinate: QtPositioning.coordinate(root.onboardStatus.latitude, root.onboardStatus.longitude)
                        anchorPoint {
                            x: icon.width / 2
                            y: root.onboardStatus.hasHeading ? icon.height / 2 : icon.height
                        }
                        visible: root.onboardStatus.hasPosition
                        sourceItem: Item {
                            Kirigami.Theme.colorSet: Kirigami.Theme.Selection
                            Kirigami.Theme.inherit: false
                            Kirigami.Icon {
                                id: icon
                                source: root.onboardStatus.hasHeading ? "qrc:///images/arrow.svg" : "map-symbolic"
                                width: height
                                height: Kirigami.Units.iconSizes.medium
                                color: Kirigami.Theme.backgroundColor
                                isMask: true
                                rotation: root.onboardStatus.hasHeading ? root.onboardStatus.heading : 0.0
                                transformOrigin: Item.Center
                                onTransformOriginChanged: icon.transformOrigin = Item.Center
                            }
                            Rectangle {
                                anchors.top: icon.bottom
                                anchors.horizontalCenter: icon.horizontalCenter
                                color: Kirigami.Theme.backgroundColor
                                width: layout.implicitWidth
                                height: layout.implicitHeight
                                RowLayout {
                                    id: layout
                                    Kirigami.Icon {
                                        source: "speedometer"
                                        isMask: true
                                        Layout.preferredHeight: Kirigami.Units.gridUnit
                                        Layout.preferredWidth: Kirigami.Units.gridUnit
                                        visible: root.onboardStatus.hasSpeed
                                    }
                                    QQC2.Label {
                                        text: Localizer.formatSpeed(root.onboardStatus.speed, Settings.distanceFormat)
                                        visible: root.onboardStatus.hasSpeed
                                    }
                                    Kirigami.Icon {
                                        source: "arrow-up-symbolic"
                                        isMask: true
                                        Layout.preferredHeight: Kirigami.Units.gridUnit
                                        Layout.preferredWidth: Kirigami.Units.gridUnit
                                        visible: root.onboardStatus.hasAltitude
                                    }
                                    QQC2.Label {
                                        text: Localizer.formatAltitude(root.onboardStatus.altitude)
                                        visible: root.onboardStatus.hasAltitude
                                    }
                                }
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
                z: map.z + 10
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
            journeySection: root.onboardStatus.journey.sections[0]
            showProgress: true
            enableMapView: false

            Kirigami.PlaceholderMessage {
                anchors.fill: parent
                visible: !root.onboardStatus.hasJourney
                text: i18n("Waiting for data…")
            }
        }
    }

    footer: Kirigami.NavigationTabBar {
        id: footerTabBar
        visible: root.onboardStatus.supportsPosition && root.onboardStatus.supportsJourney

        actions: [
            Kirigami.Action {
                id: positionAction
                text: i18n("Position")
                icon.name: 'map-symbolic'
                onTriggered: swipeView.currentIndex = 0
                checked: swipeView.currentIndex === 0
                enabled: root.onboardStatus.hasPosition
                visible: root.onboardStatus.supportsPosition
            },
            Kirigami.Action {
                id: journeyAction
                text: i18n("Journey")
                icon.name: 'view-calendar-day'
                onTriggered: swipeView.currentIndex = 1;
                checked: swipeView.currentIndex === 1
                enabled: root.onboardStatus.hasJourney
                visible: root.onboardStatus.supportsJourney
            }
        ]
    }
}
