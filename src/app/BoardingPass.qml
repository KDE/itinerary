/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

Item {
    id: root
    property KPkPass.BoardingPass pass: null
    property string passId
    property int __margin: 10

    implicitWidth: Math.max(bodyLayout.implicitWidth, 332)

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    ColumnLayout {
        id: topLayout
        spacing: 0
        width: parent.implicitWidth
        // HACK to break binding loop on implicitHeight
        onImplicitHeightChanged: root.implicitHeight = implicitHeight

        GridLayout {
            id: headerLayout
            rows: 2
            columns: root.pass.headerFields.length + 2
            Layout.fillWidth: true
            Layout.margins: root.__margin

            Image {
                Layout.rowSpan: 2
                Layout.maximumHeight: 60
                Layout.maximumWidth: 150
                Layout.preferredWidth: paintedWidth
                fillMode: Image.PreserveAspectFit
                source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/logo" : ""
                sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
            }

            QQC2.Label {
                Layout.rowSpan: 2
                Layout.fillWidth: root.pass ? true : false
                text: root.pass ? root.pass.logoText : ""
                color: root.pass.foregroundColor
            }

            Repeater {
                model: root.pass.headerFields
                delegate: QQC2.Label {
                    required property KPkPass.field modelData
                    text: modelData.label
                    color: root.pass.labelColor
                }
            }
            Repeater {
                model: root.pass.headerFields
                delegate: QQC2.Label {
                    required property KPkPass.field modelData
                    text: modelData.valueDisplayString
                    color: root.pass.foregroundColor
                }
            }
        }

        ColumnLayout {
            id: bodyLayout
            Layout.margins: root.__margin
            spacing: root.__margin

            // primary fields
            GridLayout {
                id: primaryFieldsLayout
                rows: 2
                columns: 3
                flow: GridLayout.TopToBottom
                Layout.fillWidth: true

                QQC2.Label {
                    id: primaryLabel
                    Layout.fillWidth: true
                    Layout.preferredWidth: primaryFieldsLayout.width/2 - Kirigami.Units.iconSizes.smallMedium
                    text: root.pass.primaryFields[0].label
                    color: root.pass.labelColor
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    id: primaryValue
                    text: root.pass.primaryFields[0].valueDisplayString
                    color: root.pass.foregroundColor
                    font.pointSize: 1.5 * primaryLabel.font.pointSize
                }

                Kirigami.Icon {
                    Layout.rowSpan: 2
                    Layout.alignment: Qt.AlignBottom
                    source: {
                        switch (root.pass.transitType) {
                            case KPkPass.BoardingPass.Air: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Air)
                            case KPkPass.BoardingPass.Boat: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Ferry)
                            case KPkPass.BoardingPass.Bus: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Bus)
                            case KPkPass.BoardingPass.Train: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Train)
                        }
                        return "go-next-symbolic";
                    }
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    color: root.pass.labelColor
                    isMask: true
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.preferredWidth: primaryFieldsLayout.width/2 - Kirigami.Units.iconSizes.smallMedium
                    horizontalAlignment: Qt.AlignRight
                    text: root.pass.primaryFields[1].label
                    color: root.pass.labelColor
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    Layout.alignment: Qt.AlignRight
                    text: root.pass.primaryFields[1].valueDisplayString
                    color: root.pass.foregroundColor
                    font.pointSize: primaryValue.font.pointSize
                }
            }

            // auxiliary fields
            GridLayout {
                id: auxFieldsLayout
                rows: 2
                columns: root.pass.auxiliaryFields.length
                Layout.fillWidth: true

                Repeater {
                    model: root.pass.auxiliaryFields
                    delegate: QQC2.Label {
                        required property KPkPass.field modelData
                        Layout.fillWidth: true
                        color: root.pass.labelColor
                        text: modelData.label
                        horizontalAlignment: modelData.textAlignment
                    }
                }
                Repeater {
                    model: root.pass.auxiliaryFields
                    delegate: QQC2.Label {
                        required property KPkPass.field modelData
                        Layout.fillWidth: true
                        color: root.pass.foregroundColor
                        text: modelData.valueDisplayString
                        horizontalAlignment: modelData.textAlignment
                    }
                }
            }

            // secondary fields
            GridLayout {
                id: secFieldsLayout
                rows: 2
                columns: root.pass.secondaryFields.length
                Layout.fillWidth: true

                Repeater {
                    model: root.pass.secondaryFields
                    delegate: QQC2.Label {
                        required property KPkPass.field modelData
                        Layout.fillWidth: true
                        color: root.pass.labelColor
                        text: modelData.label
                        horizontalAlignment: modelData.textAlignment
                    }
                }
                Repeater {
                    model: root.pass.secondaryFields
                    delegate: QQC2.Label {
                        required property KPkPass.field modelData
                        Layout.fillWidth: true
                        color: root.pass.foregroundColor
                        text: modelData.valueDisplayString
                        horizontalAlignment: modelData.textAlignment
                    }
                }
            }

            // footer
            Image {
                source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/footer" : ""
                sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                Layout.alignment: Qt.AlignCenter
                Layout.maximumWidth: 312
                fillMode: Image.PreserveAspectFit
            }
        }

        // barcode
        PkPassBarcode {
            maximumWidth: root.implicitWidth * 0.8
            pass: root.pass
            TapHandler {
                onDoubleTapped: root.scanModeToggled()
            }
        }

        ColumnLayout {
            id: backLayout
            Layout.margins: root.__margin

            Kirigami.Separator {
                visible: root.pass.backFields.length > 0
                Layout.fillWidth: true
            }
            Repeater {
                model: root.pass.backFields
                ColumnLayout {
                    id: delegateRoot
                    required property KPkPass.field modelData
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: root.pass.labelColor
                        text: delegateRoot.modelData.label
                        wrapMode: Text.WordWrap
                        horizontalAlignment: delegateRoot.modelData.textAlignment
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: root.pass.foregroundColor
                        linkColor: color
                        text: Util.textToHtml(delegateRoot.modelData.valueDisplayString)
                        textFormat: Util.isRichText(delegateRoot.modelData.valueDisplayString) ? Text.StyledText : Text.AutoText
                        wrapMode: Text.WordWrap
                        horizontalAlignment: delegateRoot.modelData.textAlignment
                        onLinkActivated: (link) =>  { Qt.openUrlExternally(link); }
                    }
                }
            }
        }
    }

    ColumnLayout {
        id: backgroundLayout
        anchors.fill: topLayout
        spacing: 0
        z: -1

        Rectangle {
            id: headerBackground
            radius: root.__margin
            color: root.pass.backgroundColor
            Layout.fillWidth: true
            implicitHeight: headerLayout.implicitHeight + 2 * root.__margin
        }

        Rectangle {
            id: bodyBackground
            radius: root.__margin
            color: root.pass.backgroundColor
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
