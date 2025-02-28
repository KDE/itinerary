/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.coreaddons as CoreAddons
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

Kirigami.Page {
    id: root
    property alias path: pathModel.path

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    padding: 0

    Component {
        id: pathDelegate
        QQC2.ItemDelegate {
            highlighted: false
            width: ListView.view.width
            property var section: model.section
            contentItem: GridLayout {
                rows: 2
                columns: 4

                Kirigami.Icon {
                    Layout.row: 0
                    Layout.column: 0
                    Layout.rowSpan: 2
                    source: section.iconName
                    width: height
                    height: Kirigami.Units.iconSizes.medium
                    isMask: true
                }

                // floor level change indicator
                Kirigami.Icon {
                    Layout.row: 0
                    Layout.column: 1
                    Layout.rowSpan: 2
                    width: height
                    height: Kirigami.Units.iconSizes.medium
                    source: section.floorLevelChange == 0 ? "" : section.floorLevelChange < 0 ? "go-down-skip" : "go-up-skip"
                }

                QQC2.Label {
                    Layout.row: 0
                    Layout.column: 2
                    Layout.fillWidth: true
                    text: section.description
                }
                QQC2.Label {
                    Layout.row: 1
                    Layout.column: 2
                    visible: section.distance > 0
                    text: CoreAddons.Format.formatDistance(section.distance, Settings.distanceFormat)
                }

                // direction indicator
                Kirigami.Icon {
                    Layout.row: 0
                    Layout.column: 3
                    Layout.rowSpan: 2
                    width: height
                    height: Kirigami.Units.iconSizes.medium
                    source: "go-up-symbolic"
                    visible: model.turnDirection >= 0
                    rotation: model.turnDirection
                    smooth: true
                }
            }
        }
    }

    KPublicTransport.PathModel {
        id: pathModel
    }

    ColumnLayout{
        anchors.fill: parent
        spacing: 0

        QQC2.SwipeView {
            id: view
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            Item {
                id: listPage
                QQC2.ScrollView {
                    id: scrollview
                    anchors.fill: parent
                    ListView {
                        model: pathModel
                        delegate: pathDelegate
                    }
                }
            }
            Item {
                id: mapPage
                MapView {
                    id: map
                    anchors.fill: parent
                    QtLocation.MapPolyline {
                        id: line
                        line.width: 6
                        // hardcoded Breeze black, can't use Kirigami theme colors as we need contrast to OSM tiles here, also in dark mode
                        line.color: "#232629"
                        path: KPublicTransport.MapUtils.polyline(root.path);
                    }

                    MapCircle {
                        coordinate {
                           latitude: root.path.startPoint.y
                           longitude: root.path.startPoint.x
                        }
                        color: line.line.color
                        textColor: "#eff0f1"
                        size: 15
                    }

                    Repeater {
                        model: root.path.sections
                        MapCircle {
                            coordinate {
                                latitude: modelData.startPoint.y
                                longitude: modelData.startPoint.x
                            }
                            color: line.line.color
                            textColor: "#eff0f1"
                            visible: map.zoomLevel > 17
                            size: map.zoomLevel > 19 ? 22 : 16
                            rotation: modelData.maneuver == KPublicTransport.PathSection.Move ? modelData.direction : 0
                            iconName: modelData.maneuver == KPublicTransport.PathSection.Move ? "go-up" : modelData.iconName
                        }
                    }

                    MapCircle {
                        coordinate {
                           latitude: root.path.endPoint.y
                           longitude: root.path.endPoint.x
                        }
                        color: line.line.color
                        textColor: "#eff0f1"
                        size: 15
                    }
                }

                function centerOnPath() {
                    const bbox = KPublicTransport.MapUtils.boundingBox(root.path);
                    map.center = KPublicTransport.MapUtils.center(bbox);
                    map.zoomLevel = KPublicTransport.MapUtils.zoomLevel(bbox, map.width, map.height);
                }

                onWidthChanged: centerOnPath();
                onHeightChanged: centerOnPath();
                Component.onCompleted: centerOnPath();
            }
        }
    }

    Components.FloatingButton {
        id: mapButton
        icon.name: checked ? "format-list-unordered" : "map-gnomonic"
        text: i18nc("@action:button", "Show Map")
        onClicked: view.currentIndex === 0 ? view.currentIndex = 1 : view.currentIndex = 0
        checkable: true
        checked: view.currentIndex === 1

        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (scrollview.QQC2.ScrollBar && scrollview.QQC2.ScrollBar.vertical ? scrollview.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
    }
}
