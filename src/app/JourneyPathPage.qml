/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    property alias path: pathModel.path

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
                    text: Localizer.formatDistance(section.distance)
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

    PathModel {
        id: pathModel
    }

    ListView {
        model: pathModel
        delegate: pathDelegate
    }
}
