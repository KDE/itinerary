/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Effects as Effects
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass
import org.kde.itinerary

Item {
    id: root
    property KPkPass.Pass pass: null
    property string passId
    implicitHeight: bodyBackground.implicitHeight
    implicitWidth: 332

    property color defaultTextColor: Kirigami.Theme.textColor

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    Rectangle {
        id: bodyBackground
        color: root.pass.hasBackgroundColor ? root.pass.backgroundColor : Kirigami.Theme.backgroundColor
        width: parent.width

        Image {
            id: backgroundImage
            source: root.passId !== "" ? (root.pass.hasArtwork ? "image://org.kde.pkpass/" + root.passId + "/artwork" : "image://org.kde.pkpass/" + root.passId + "/background") : ""
            fillMode: backgroundImage.implicitHeight < parent.implicitHeight ? Image.TileVertically : Image.PreserveAspectCrop
            verticalAlignment: Image.AlignTop
            horizontalAlignment: Image.AlignHCenter
            x: -(width - bodyBackground.width) / 2
            y: 0
            visible: false
            height: parent.implicitHeight
            width: root.implicitWidth
        }
        Effects.MultiEffect {
            anchors.fill: backgroundImage
            source: backgroundImage
            autoPaddingEnabled: false
            blurEnabled: true // TODO this should only be applied to the bottom part
            blur: 1.0
            blurMax: 32
        }

        ColumnLayout {
            id: topLayout
            spacing: 10
            anchors.fill: parent
            anchors.margins: 6
            // HACK to break binding loop on implicitHeight
            onImplicitHeightChanged: bodyBackground.implicitHeight = Math.max(implicitHeight + 2 * topLayout.anchors.margins, 448)

            // header
            PosterEventTicketHeader {
                pass: root.pass
                passId: root.passId
                defaultTextColor: root.defaultTextColor
            }

            Item { Layout.fillHeight: true }

            // barcode
            Barcode {
                maximumWidth: root.implicitWidth * 0.8
                pass: root.pass
                TapHandler {
                    onDoubleTapped: root.scanModeToggled()
                }
            }

            // seat information
            RowLayout {
                QQC2.Label {
                    color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
                    text: root.pass.semanticTags.admissionLevel
                    // TODO larger font
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Repeater {
                        model: root.pass.seats
                        delegate: PosterEventTicketSeatRow {
                            required property KPkPass.seat modelData
                            pass: root.pass
                            seat: modelData
                            defaultTextColor: root.defaultTextColor
                        }
                    }
                }
            }

            // footer
            PosterEventTicketFooter {
                pass: root.pass
                passId: root.passId
                defaultTextColor: root.defaultTextColor
            }

            // back fields
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: root.pass.backFields.length > 0
            }
            BackFields {
                pass: root.pass
                defaultTextColor: root.defaultTextColor
                Layout.fillHeight: false
            }
        }

        Rectangle {
            id: notchMask
            width: parent.width / 4
            height: width
            radius: width / 2
            color: Kirigami.Theme.backgroundColor
            x: parent.width/2 - radius
            y: -radius * 1.5
        }
    }

    Component.onCompleted: {
        if (backgroundImage.status !== Image.Ready)
            return;
        root.defaultTextColor = Util.isDarkImage(root.pass.background()) ? "#eff0f1" : "#232629";
    }
}
