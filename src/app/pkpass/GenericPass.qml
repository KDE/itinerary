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

Item {
    id: root
    property KPkPass.Pass pass: null
    property string passId
    implicitHeight: Math.max(implicitHeight + 2 * topLayout.anchors.margins, 448)
    implicitWidth: 332 //Math.max(topLayout.implicitWidth, 332)


    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

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
        GridLayout {
            id: headerLayout
            rows: 2
            columns: root.pass.headerFields.length + 2
            Layout.fillWidth: true
            Layout.maximumWidth: root.implicitWidth - 2 * topLayout.anchors.margins

            Image {
                Layout.rowSpan: 2
                Layout.maximumHeight: 50
                Layout.maximumWidth: 160
                Layout.preferredWidth: paintedWidth
                fillMode: Image.PreserveAspectFit
                source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/logo" : ""
                sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
            }

            QQC2.Label {
                Layout.rowSpan: 2
                Layout.fillWidth: root.pass ? true : false
                text: root.pass ? root.pass.logoText : ""
                color: root.pass.hasForegroundColor ? root.pass.foregroundColor : Kirigami.Theme.textColor
            }

            Repeater {
                model: root.pass.headerFields
                delegate: QQC2.Label {
                    required property KPkPass.field modelData
                    text: modelData.label
                    color: root.pass.hasLabelColor ? root.pass.labelColor : Kirigami.Theme.textColor
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            Repeater {
                model: root.pass.headerFields
                delegate: QQC2.Label {
                    required property KPkPass.field modelData
                    text: modelData.valueDisplayString
                    color: root.pass.hasForegroundColor ? root.pass.foregroundColor : Kirigami.Theme.textColor
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
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
                    color: root.pass.hasLabelColor ? root.pass.labelColor : Kirigami.Theme.textColor
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
                    color: root.pass.hasForegroundColor ? root.pass.foregroundColor : Kirigami.Theme.textColor
                    text: modelData.valueDisplayString
                    horizontalAlignment: modelData.textAlignment
                }
            }
        }

        // secondary fields
        SecondaryFieldsRow {
            pass: root.pass
            defaultTextColor: Kirigami.Theme.textColor
        }

        // auxiliary fields
        AuxiliaryFieldsGrid {
            pass: root.pass
            defaultTextColor: Kirigami.Theme.textColor
        }

        // barcode
        Barcode {
            maximumWidth: root.implicitWidth * 0.8
            pass: root.pass
            TapHandler {
                onDoubleTapped: root.scanModeToggled()
            }
        }

        // back fields
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: root.pass.backFields.length > 0
        }
        BackFields {
            pass: root.pass
            defaultTextColor: Kirigami.Theme.textColor
        }
    }
}
