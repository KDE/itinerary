/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as PublicTransport
import org.kde.kpublictransport.ui as PublicTransport
import org.kde.kosmindoormap
import org.kde.itinerary

Kirigami.Dialog {
    id: platformSheet

    property var model

    title: i18nc("@title", "Find Platform")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        model: platformSheet.model
        clip: true

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
                        delegate: PublicTransport.TransportIcon {
                            iconHeight: Kirigami.Units.iconSizes.small
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
                    map.view.centerOn(model.coordinate, model.level, 19);
                    platformSheet.close();
                }
            }
        }

        section.property: "mode"
        section.delegate: Kirigami.ListSectionHeader {
            text: PlatformUtil.modeName(parseInt(section))
            width: ListView.view.width
        }
        section.criteria: ViewSection.FullString

        delegate: platformDelegate
    }
}
