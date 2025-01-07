// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport

/** Visual representation of a journey section in a horizontal bar display.
 *  Also usable as a stand-alone transport line indicator.
 *
 *  Public transport sections are shown in line colors (or inverted text colors if none available),
 *  all other section modes are shown in regular text colors without background.
 */
QQC2.Control {
    id: root

    /** The journey section to represent.
     *  Setting this (and leaving all other properties untouched) is the easiest and preferred
     *  way of using this.
     *
     *  If set all other properties are populated from this automatically.
     *  If not set, at least line and journeySectionMode need to be specified instead.
     */
    property KPublicTransport.journeySection journeySection

    /** Line information of the journey section. */
    property KPublicTransport.line line: journeySection.route.line

    /** Journey section mode.
     *  @see KPublicTransport::JourneySection::Mode
     */
    property int journeySectionMode: root.journeySection.mode === KPublicTransport.JourneySection.Invalid ? KPublicTransport.JourneySection.PublicTransport : root.journeySection.mode

    /** Description of the journey section mode.
     *  Not displayed in the UI but used as an a11y text for mode icons.
     */
    property string modeName: root.journeySection.label

    /** Icon representing the mode of transport or the line logo. */
    property string iconName: root.journeySection.mode === KPublicTransport.JourneySection.Invalid ? root.line.iconName : root.journeySection.iconName
    /** Displayed name of the public transport line. */
    property string lineName: root.line.name

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

        KPublicTransport.TransportIcon {
            id: transportIcon
            color: if (background.visible) {
                return lineName.color;
            } else {
                Kirigami.Theme.textColor
            }
            isMask: true
            iconHeight: Kirigami.Units.iconSizes.smallMedium
            source: root.iconName
            Accessible.name: root.modeName

            Layout.rightMargin: lineName.visible ? Kirigami.Units.smallSpacing : 0
        }

        Kirigami.Heading {
            id: lineName

            readonly property bool isDarkTheme: Kirigami.Theme.backgroundColor.hslLightness < 0.4

            color: {
                const backgroundIsDark = background.color.hslLightness < 0.4;
                if (root.line.hasTextColor) {
                    const foregroundIsLight = root.line.textColor.hslLightness >= 0.4;
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
            visible: text.length > 0 && root.journeySectionMode === KPublicTransport.JourneySection.PublicTransport && !root.line.hasLogo
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
        visible: root.journeySectionMode === KPublicTransport.JourneySection.PublicTransport
    }
}
