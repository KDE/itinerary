/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kopeninghours 1.0
import org.kde.kosmindoormap 1.0

Kirigami.AbstractListItem {
    id: root
    required property string name
    required property string typeName
    required property string iconSource
    required property string cuisine
    required property string fallbackName
    required property string openingHours

    required property var mapData

    required property int index // for Kirigami

    highlighted: false

    property var oh: {
        let v = OpeningHoursParser.parse(root.openingHours);
        v.region = root.mapData.regionCode;
        v.timeZone = root.mapData.timeZone;
        v.setLocation(root.mapData.center.y, root.mapData.center.x);
        if (v.error != OpeningHours.Null && v.error != OpeningHours.NoError) {
            console.log("Opening hours parsing error:", v.error, root.mapData.region, root.mapData.timeZone)
        }
        return v;
    }

    RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.minimumHeight: Kirigami.Units.iconSizes.medium
            Layout.maximumHeight: Kirigami.Units.iconSizes.medium
            Layout.minimumWidth: Kirigami.Units.iconSizes.medium
            Layout.maximumWidth: Kirigami.Units.iconSizes.medium
            source: root.iconSource
            isMask: true
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            spacing: 0

            QQC2.Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                Layout.fillWidth: true
                text: root.name !== "" ? root.name : root.typeName
                elide: Text.ElideRight
            }

            QQC2.Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                text: {
                    if (root.cuisine && root.name === "")
                        return root.cuisine;
                    if (root.cuisine)
                        return root.typeName + " (" + root.cuisine + ")";
                    return root.name === "" && root.fallbackName !== "" ? root.fallbackName : root.typeName;
                }
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                opacity: 0.7
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: Display.currentState(root.oh)
                color: {
                    if (root.highlighted || root.checked || root.down)
                        return Kirigami.Theme.highlightedTextColor
                    const currentInterval = root.oh.interval(new Date());
                    switch (currentInterval.state) {
                        case Interval.Open: return Kirigami.Theme.positiveTextColor;
                        case Interval.Closed: return Kirigami.Theme.negativeTextColor;
                        default: return Kirigami.Theme.textColor;
                    }
                }
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                visible: text !== ""
            }
        }
    }
}
