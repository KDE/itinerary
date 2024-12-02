// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport

QQC2.Control {
    id: root

    required property KPublicTransport.line line
    required property int mode
    property string modeName
    property string iconName
    property string lineName: line.name

    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    clip: true

    contentItem: RowLayout {
        spacing: 0

        Item {
            Layout.fillWidth: true
        }

        TransportIcon {
            id: transportIcon
            color: if (background.visible) {
                return lineName.color;
            } else {
                Kirigami.Theme.textColor
            }
            isMask: true
            size: Kirigami.Units.iconSizes.smallMedium
            source: root.iconName
            Accessible.name: root.modeName

            Layout.rightMargin: lineName.visible ? Kirigami.Units.smallSpacing : 0
        }

        Kirigami.Heading {
            id: lineName

            function getDarkness(background: color): real {
                const temp = Qt.darker(background, 1);
                const a = 1 - (0.299 * temp.r + 0.587 * temp.g + 0.114 * temp.b);
                return a;
            }

            readonly property bool isDarkTheme: {
                const temp = Qt.darker(Kirigami.Theme.backgroundColor, 1);
                return temp.a > 0 && getDarkness(Kirigami.Theme.backgroundColor) >= 0.4;
            }

            color: {
                const backgroundIsDark = getDarkness(background.color) >= 0.4;
                if (root.line.hasTextColor) {
                    const foregroundIsLight = getDarkness(root.line.textColor) < 0.4;
                    if (foregroundIsLight === backgroundIsDark) {
                        return root.line.textColor;
                    }
                    // fallthrough and use our own color has the line text color doesn't offer enough contrast
                    // Test case U4 in Berlin
                }

                return backgroundIsDark && isDarkTheme ? Kirigami.Theme.textColor : Kirigami.Theme.backgroundColor;
            }
            level: 4
            text: root.lineName
            visible: text.length > 0 && root.mode === KPublicTransport.JourneySection.PublicTransport && !root.line.hasLogo
            elide: Text.ElideRight
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            Layout.minimumHeight: Kirigami.Units.iconSizes.smallMedium
            Layout.fillHeight: true
        }

        Item {
            Layout.fillWidth: true
        }
    }

    background: Rectangle {
        id: background

        radius: Kirigami.Units.cornerRadius
        color: root.line.hasColor ? root.line.color : Kirigami.Theme.textColor
        visible: root.mode === KPublicTransport.JourneySection.PublicTransport
    }
}
