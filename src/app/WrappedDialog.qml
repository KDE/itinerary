// SPDX-CopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Controls.Dialog {
    id: root

    modal: true
    header: Kirigami.Heading {
        text: i18nc("@title:dialog", "Itinerary Wrapped %1 available", "" + (new Date()).getFullYear())
        wrapMode: Text.WordWrap
        padding: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.largeSpacing * 2
        horizontalAlignment: Text.AlignHCenter
    }

    Controls.Button {
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false

        anchors.horizontalCenter: parent.horizontalCenter
        text: i18nc("@action:button", "Explore")
        onClicked: {
            root.Controls.ApplicationWindow.window.pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "WrappedPage"))
            root.close();
        }
    }

    background: Item {
        Kirigami.ShadowedRectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height)
            height: width
            radius: controlRoot.radius

            shadow {
                size: 10
                xOffset: 0
                yOffset: 2
                color: Qt.rgba(0, 0, 0, 0.2)
            }

            border {
                width: 1
                color: if (controlRoot.down || controlRoot.visualFocus) {
                    Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.4)
                } else if (controlRoot.enabled && controlRoot.hovered) {
                    Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6)
                } else {
                    Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
                }
            }

            color: if (controlRoot.down || controlRoot.visualFocus) {
                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6)
            } else if (controlRoot.enabled && controlRoot.hovered) {
                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8)
            } else {
                Kirigami.Theme.backgroundColor
            }

            Behavior on border.color {
                ColorAnimation {
                    duration: Kirigami.Units.veryShortDuration
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: Kirigami.Units.veryShortDuration
                }
            }
        }
    }

    footer: Image {
        height: 160
        sourceSize.height: 160
        width: parent.width
        source: Qt.resolvedUrl("wrapped-banner.svg")
        fillMode: Image.PreserveAspectCrop
    }
}
