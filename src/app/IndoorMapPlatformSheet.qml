/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as PublicTransport
import org.kde.kosmindoormap
import org.kde.itinerary

Kirigami.OverlaySheet {
    id: platformSheet
    property var model

    header: Kirigami.Heading {
        text: i18n("Find Platform")
    }

    ListView {
        model: platformSheet.model
        clip: true
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25

        Component {
            id: platformDelegate
            QQC2.ItemDelegate {
                property var platform: model
                width: ListView.view.width
                contentItem: Row {
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        id: label
                        text: {
                            if (platform.isDeparturePlatform && platform.isArrivalPlatform)
                                return i18nc("train arrival/departure platform", "%1 (arrival/departure)", platform.display);
                            if (platform.isDeparturePlatform)
                                return i18nc("train departure platform", "%1 (departure)", platform.display);
                            if (platform.isArrivalPlatform)
                                return i18nc("train arrival platform", "%1 (arrival)", platform.display);
                            return platform.display
                        }
                    }

                    Repeater {
                        model: platform.lines
                        delegate: Image {
                            height: Kirigami.Units.iconSizes.small
                            width: Math.round(Kirigami.Units.iconSizes.small * implicitWidth / implicitHeight)
                            anchors.verticalCenter: label.verticalCenter
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
                    platformSheet.close();
                }
            }
        }

        section.property: "mode"
        section.delegate: Kirigami.ListSectionHeader {
            text: switch(parseInt(section)) {
                case Platform.Rail: return i18n("Railway");
                case Platform.LightRail: return i18n("Light Rail");
                case Platform.Subway: return i18n("Subway");
                case Platform.Tram: return i18n("Tramway");
                case Platform.Bus: return i18n("Bus");
                default: return section;
            }
            width: ListView.view.width
        }
        section.criteria: ViewSection.FullString

        delegate: platformDelegate
    }
}
