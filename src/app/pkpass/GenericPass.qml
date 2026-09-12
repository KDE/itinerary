/*
    SPDX-FileCopyrightText: 2018-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass

AbstractPass {
    id: root
    implicitHeight: Math.max(implicitHeight + 2 * topLayout.anchors.margins, 448)
    //implicitWidth: Math.max(topLayout.implicitWidth, 332)

    GenericPassBackground {
        pass: root.pass
        passId: root.passId
        anchors.fill: parent
    }

    ColumnLayout {
        id: topLayout
        spacing: 10
        anchors.fill: parent
        anchors.margins: 6
        // HACK to break binding loop on implicitHeight
        onImplicitHeightChanged: root.implicitHeight = Math.max(implicitHeight + 2 * topLayout.anchors.margins, 448)

        // header
        GenericPassHeader {
            id: headerLayout
            pass: root.pass
            passId: root.passId
            defaultTextColor: root.defaultTextColor
            Layout.maximumWidth: root.implicitWidth - 2 * topLayout.anchors.margins
        }

        // primary fields
        Kirigami.Separator {
            Layout.fillWidth: true
        }
        GridLayout {
            id: primaryFieldLayout
            rows: 2
            columns: root.pass.primaryFields.length + 1
            Layout.fillWidth: true

            Repeater {
                model: root.pass.primaryFields
                delegate: QQC2.Label {
                    required property KPkPass.field modelData
                    Layout.fillWidth: true
                    color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
                    text: modelData.label
                    horizontalAlignment: modelData.textAlignment
                }
            }
            Image {
                id: thumbnailImage
                Layout.rowSpan: 2
                source: "image://org.kde.pkpass/" + root.passId + "/thumbnail"
                sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: Math.min(90, thumbnailImage.implicitWidth); // 90x90 as per spec
                Layout.preferredHeight: (Layout.preferredWidth / thumbnailImage.implicitWidth) * thumbnailImage.implicitHeight
                fillMode: Image.PreserveAspectFit
            }
            Repeater {
                model: root.pass.primaryFields
                delegate: QQC2.Label {
                    required property KPkPass.field modelData
                    Layout.fillWidth: true
                    color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
                    text: modelData.valueDisplayString
                    horizontalAlignment: modelData.textAlignment
                }
            }
        }

        // secondary fields
        SecondaryFieldsRow {
            pass: root.pass
            defaultTextColor: root.defaultTextColor
        }

        // auxiliary fields
        AuxiliaryFieldsGrid {
            pass: root.pass
            defaultTextColor: root.defaultTextColor
        }

        // barcode
        Barcode {
            maximumWidth: root.implicitWidth * 0.8
            pass: root.pass
            TapHandler {
                onDoubleTapped: root.barcodeDoubleTapped()
            }
        }

        // back fields
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: root.pass.backFields.length > 0
        }
        BackFields {
            pass: root.pass
            defaultTextColor: root.defaultTextColor
        }
    }
}
