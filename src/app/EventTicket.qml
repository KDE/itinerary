/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Effects as Effects
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass
import org.kde.itinerary

Item {
    id: root
    property var pass: null
    property string passId
    implicitHeight: bodyBackground.implicitHeight
    implicitWidth: 332 //Math.max(topLayout.implicitWidth, 332)

    property color defaultTextColor: Kirigami.Theme.textColor

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    Rectangle {
        id: bodyBackground
        color: Util.isValidColor(pass.backgroundColor) ? pass.backgroundColor : Kirigami.Theme.backgroundColor
        //implicitHeight: topLayout.implicitHeight + 2 * topLayout.anchors.margins
        width: parent.width

        Image {
            id: backgroundImage
            source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/background" : ""
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
            blurEnabled: true
            blur: 1.0
            blurMax: 32
        }

        ColumnLayout {
            id: topLayout
            spacing: 10
            anchors.fill: parent
            anchors.margins: 6
            // HACK to break binding loop on implicitHeight
            onImplicitHeightChanged: bodyBackground.implicitHeight = implicitHeight + 2 * topLayout.anchors.margins


            ColumnLayout {
                id: bodyLayout
                spacing: 10

                // header
                GridLayout {
                    id: headerLayout
                    rows: 2
                    columns: pass.headerFields.length + 2
                    Layout.fillWidth: true

                    Image {
                        Layout.rowSpan: 2
                        Layout.maximumHeight: 60
                        Layout.maximumWidth: 150
                        Layout.preferredWidth: paintedWidth
                        fillMode: Image.PreserveAspectFit
                        source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/logo" : ""
                        sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                        mipmap: true
                    }

                    QQC2.Label {
                        Layout.rowSpan: 2
                        Layout.fillWidth: pass ? true : false
                        text: pass ? pass.logoText : ""
                        color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : root.defaultTextColor
                    }

                    Repeater {
                        model: pass.headerFields
                        delegate: QQC2.Label {
                            text: modelData.label
                            color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : root.defaultTextColor
                            Layout.row: 0
                            Layout.column: index + 2
                        }
                    }
                    Repeater {
                        model: pass.headerFields
                        delegate: QQC2.Label {
                            text: modelData.valueDisplayString
                            color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : root.defaultTextColor
                            Layout.row: 1
                            Layout.column: index + 2
                        }
                    }
                }

                // strip image
                Image {
                    id: stripImage
                    source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/strip" : ""
                    sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: Math.min(320, stripImage.implicitWidth); // 320 as per spec
                    Layout.preferredHeight: (Layout.preferredWidth / stripImage.implicitWidth) * stripImage.implicitHeight
                    fillMode: Image.PreserveAspectFit
                }

                // primary fields
                 GridLayout {
                    id: primaryFieldLayout
                    rows: 2
                    columns: pass.primaryFields.length
                    Layout.fillWidth: true

                    Repeater {
                        model: pass.primaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : root.defaultTextColor
                            text: modelData.label
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                    Repeater {
                        model: pass.primaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : root.defaultTextColor
                            text: modelData.valueDisplayString
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                }

                // secondary fields
                GridLayout {
                    id: secFieldsLayout
                    rows: 2
                    columns: pass.secondaryFields.length
                    Layout.fillWidth: true

                    Repeater {
                        model: pass.secondaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : root.defaultTextColor
                            text: modelData.label
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                    Repeater {
                        model: pass.secondaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : root.defaultTextColor
                            text: modelData.valueDisplayString
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                }

                // auxiliary fields
                GridLayout {
                    id: auxFieldsLayout
                    rows: 2
                    columns: pass.auxiliaryFields.length
                    Layout.fillWidth: true

                    Repeater {
                        model: pass.auxiliaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : root.defaultTextColor
                            text: modelData.label
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                    Repeater {
                        model: pass.auxiliaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : root.defaultTextColor
                            text: modelData.valueDisplayString
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                }
            }

            // barcode
            PkPassBarcode {
                maximumWidth: root.implicitWidth * 0.8
                pass: root.pass
                TapHandler {
                    onDoubleTapped: scanModeToggled()
                }
            }

            // footer
            Image {
                source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/footer" : ""
                sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                Layout.alignment: Qt.AlignCenter
            }

            // back fields
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: pass.backFields.length > 0
            }
            Repeater {
                model: pass.backFields
                ColumnLayout {
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : root.defaultTextColor
                        text: modelData.label
                        wrapMode: Text.WordWrap
                        horizontalAlignment: modelData.textAlignment
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : root.defaultTextColor
                        linkColor: Util.isValidColor(pass.foregroundColor) ? color : Kirigami.Theme.linkColor
                        text: Util.textToHtml(modelData.valueDisplayString)
                        textFormat: Util.isRichText(modelData.valueDisplayString) ? Text.StyledText : Text.AutoText
                        wrapMode: Text.WordWrap
                        horizontalAlignment: modelData.textAlignment
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                }
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
        root.defaultTextColor = Util.isDarkImage(pass.background()) ? "#eff0f1" : "#232629";
    }
}
