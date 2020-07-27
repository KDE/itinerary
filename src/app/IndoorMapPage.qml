/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.7 as Kirigami
import org.kde.kpublictransport 1.0 as PublicTransport
import org.kde.kosmindoormap 1.0
import org.kde.itinerary 1.0

Kirigami.Page {
    title: {
        if (map.mapLoader.isLoading || map.hasError)
            return placeName;
        if (map.floorLevels.hasName(map.view.floorLevel))
            return map.floorLevels.name(map.view.floorLevel);
        return i18n("Floor %1", map.floorLevels.name(map.view.floorLevel));
    }

    property point coordinate
    property string placeName
    property alias map: map

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    // TODO in theory we could make this conditional to having panned the map all the way to the right
    Kirigami.ColumnView.preventStealing: true

    actions {
        left: Kirigami.Action {
            iconName: "go-down-symbolic"
            enabled: map.floorLevels.hasFloorLevelBelow(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelBelow(map.view.floorLevel)
            visible: map.floorLevels.rowCount() > 1
        }
        right: Kirigami.Action {
            iconName: "go-up-symbolic"
            enabled: map.floorLevels.hasFloorLevelAbove(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelAbove(map.view.floorLevel)
            visible: map.floorLevels.rowCount() > 1
        }
    }
    contextualActions: [
        Kirigami.Action {
            id: platformAction
            text: i18n("Find Platform")
            onTriggered: platformSheet.sheetOpen = true
            visible: !platformModel.isEmpty
        },
        Kirigami.Action {
            id: gateAction
            text: i18n("Find Gate")
            onTriggered: gateSheet.sheetOpen = true
            visible: !gateModel.isEmpty
        }
    ]

    OSMElementInformationModel {
        id: infoModel
        //debug: true
    }

    Component {
        id: infoStringDelegate
        RowLayout {
            QQC2.Label {
                visible: row.keyLabel != ""
                text: row.keyLabel + ":"
                color: row.category == OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                Layout.alignment: Qt.AlignTop
            }
            QQC2.Label {
                text: row.value
                color: row.category == OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }

    Component {
        id: infoLinkDelegate
        RowLayout {
            QQC2.Label {
                visible: row.keyLabel != ""
                text: row.keyLabel + ":"
                color: row.category == OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                Layout.alignment: Qt.AlignTop
            }
            QQC2.Label {
                text: "<a href=\"" + row.url + "\">" + row.value + "</a>"
                color: row.category == OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                onLinkActivated: Qt.openUrlExternally(link)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }

    Component {
        id: infoAddressDelegate
        QQC2.Label {
            text: Localizer.formatAddress(row.value)
        }
    }

    Component {
        id: infoOpeningHoursDelegate
        // TODO this is temporary until we have a proper parser and UI for this
        QQC2.Label {
            text: row.value.includes(";") ? row.keyLabel + ":\n  " + row.value.replace(/;\s*/g, "\n  ") : row.keyLabel + ": " + row.value
        }
    }

    Kirigami.OverlaySheet {
        id: elementDetailsSheet

        header: Kirigami.Heading {
            text: infoModel.name
        }

        ListView {
            model: infoModel

            section.property: "categoryLabel"
            section.delegate: Kirigami.Heading {
                x: Kirigami.Units.largeSpacing
                level: 4
                text: section
                color: section == "Debug" ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                height: implicitHeight + Kirigami.Units.largeSpacing
                verticalAlignment: Qt.AlignBottom
            }
            section.criteria: ViewSection.FullString
            section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels

            delegate: Loader {
                property var row: model
                x: Kirigami.Units.largeSpacing
                width: ListView.view.width - 2 * x
                sourceComponent: {
                    switch (row.type) {
                        case OSMElementInformationModel.Link:
                            return infoLinkDelegate;
                        case OSMElementInformationModel.PostalAddress:
                            return infoAddressDelegate;
                        case OSMElementInformationModel.OpeningHoursType:
                            return infoOpeningHoursDelegate;
                        case OSMElementInformationModel.String:
                        default:
                            return infoStringDelegate;
                    }
                }
            }
        }

        onSheetOpenChanged: {
            if (sheetOpen == false) {
                infoModel.clear()
            }
        }
    }

    PlatformModel {
        id: platformModel
        mapData: map.mapData
    }

    Component {
        id: platformDelegate
        Kirigami.AbstractListItem {
            property var platform: model
            Row {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label { text: platform.display }

                Repeater {
                    model: platform.lines
                    delegate: Kirigami.Icon {
                        height: Kirigami.Units.iconSizes.small
                        width: implicitWidth
                        visible: source != ""
                        source: {
                            switch (platform.mode) {
                                case Platform.Rail:
                                    return PublicTransport.LineMetaData.lookup(modelData, platform.coordinate.y, platform.coordinate.x, PublicTransport.Line.Train, true).logo;
                                case Platform.Tram:
                                    return PublicTransport.LineMetaData.lookup(modelData, platform.coordinate.y, platform.coordinate.x, PublicTransport.Line.Tramway, true).logo;
                                case Platform.Subway:
                                    return PublicTransport.LineMetaData.lookup(modelData, platform.coordinate.y, platform.coordinate.x, PublicTransport.Line.Metro, true).logo;
                            }
                            return "";
                        }
                    }
                }
            }
            highlighted: false
            onClicked: {
                map.view.floorLevel = model.level
                map.view.centerOnGeoCoordinate(model.coordinate);
                map.view.setZoomLevel(19, Qt.point(map.width / 2.0, map.height/ 2.0));
                platformSheet.sheetOpen = false
            }
        }
    }

    Kirigami.OverlaySheet {
        id: platformSheet

        header: Kirigami.Heading {
            text: i18n("Find Platform")
        }

        ListView {
            model: platformModel

            section.property: "mode"
            section.delegate: Kirigami.AbstractListItem {
                Kirigami.Heading {
                    x: Kirigami.Units.largeSpacing
                    level: 4
                    text: switch(parseInt(section)) {
                        case Platform.Rail: return i18n("Railway");
                        case Platform.Subway: return i18n("Subway");
                        case Platform.Tram: return i18n("Tramway");
                        case Platform.Bus: return i18n("Bus");
                        default: return section;
                    }
                }
                height: implicitHeight
            }
            section.criteria: ViewSection.FullString

            delegate: platformDelegate
        }
    }

    GateModel {
        id: gateModel
        mapData: map.mapData
    }

    Kirigami.OverlaySheet {
        id: gateSheet

        header: Kirigami.Heading {
            text: i18n("Find Gate")
        }

        ListView {
            model: gateModel

            delegate: Kirigami.BasicListItem {
                text: model.display
                highlighted: false
                onClicked: {
                    map.view.floorLevel = model.level
                    map.view.centerOnGeoCoordinate(model.coordinate);
                    map.view.setZoomLevel(18, Qt.point(map.width / 2.0, map.height/ 2.0));
                    gateSheet.sheetOpen = false
                }
            }
        }
    }

    IndoorMap {
        id: map
        anchors.fill: parent

        IndoorMapScale {
            map: map
            anchors.left: map.left
            anchors.top: map.top
            width: 0.3 * map.width
        }

        IndoorMapAttributionLabel {
            anchors.right: map.right
            anchors.bottom: map.bottom
        }

        onElementPicked: {
            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.sheetOpen = true;
            }
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);
}
