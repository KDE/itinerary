/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kpublictransport 1.0 as PublicTransport
import org.kde.kosmindoormap 1.0
import org.kde.itinerary 1.0

Kirigami.OverlaySheet {
    id: platformSheet
    property var model

    header: Kirigami.Heading {
        text: i18n("Find Platform")
    }

    ListView {
        model: platformSheet.model

        Component {
            id: platformDelegate
            Kirigami.AbstractListItem {
                property var platform: model
                Row {
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
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

        section.property: "mode"
        section.delegate: Kirigami.AbstractListItem {
            Kirigami.Heading {
                x: Kirigami.Units.largeSpacing
                level: 4
                text: switch(parseInt(section)) {
                    case Platform.Rail: return i18n("Railway");
                    case Platform.LightRail: return i18n("Light Rail");
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
