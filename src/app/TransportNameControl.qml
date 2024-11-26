// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport

QQC2.Control {
    id: root

    required property var line
    required property int mode
    property string modeName
    property string iconName
    property string lineName: line.name

    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Item {
            Layout.fillWidth: true
        }

        TransportIcon {
            color: if (background.visible) {
                return lineName.color;
            } else {
                Kirigami.Theme.textColor
            }
            isMask: true
            size: Kirigami.Units.iconSizes.smallMedium
            source: root.iconName
            Accessible.name: root.modeName
        }

        QQC2.Label {
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
            text: root.lineName
            visible: text.length > 0 && root.mode === JourneySection.PublicTransport && root.iconName.startsWith('qrc:')
            elide: Text.ElideRight
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter

            Layout.minimumHeight: Kirigami.Units.iconSizes.smallMedium
        }

        Item {
            Layout.fillWidth: true
        }
    }

    background: Rectangle {
        id: background

        radius: Kirigami.Units.cornerRadius
        color: root.line.hasColor ? root.line.color : Kirigami.Theme.backgroundColor
        visible: root.mode === JourneySection.PublicTransport
    }
}
