/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtGraphicalEffects 1.0 as Effects
import org.kde.kirigami 2.4 as Kirigami
import org.kde.pkpass 1.0 as KPkPass
import "." as App

Item {
    id: root
    property var pass: null
    property string passId
    implicitHeight: bodyBackground.implicitHeight
    implicitWidth: 332 //Math.max(topLayout.implicitWidth, 332)

    Rectangle {
        id: bodyBackground
        color: pass.backgroundColor
        //implicitHeight: topLayout.implicitHeight + 2 * topLayout.anchors.margins
        width: parent.width
        visible: false // shown via opacity mask shader effect below

        Image {
            id: backgroundImage
            source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/background" : ""
            fillMode: Image.PreserveAspectFit
            x: -(implicitWidth - bodyBackground.width) / 2
            y: 0
            visible: false
        }
        Effects.FastBlur {
            anchors.fill: backgroundImage
            source: backgroundImage
            radius: 32
        }

        ColumnLayout {
            id: topLayout
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10
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
                    }

                    QQC2.Label {
                        Layout.rowSpan: 2
                        Layout.fillWidth: pass ? true : false
                        text: pass ? pass.logoText : ""
                        color: pass.foregroundColor
                    }

                    Repeater {
                        model: pass.headerFields
                        delegate: QQC2.Label {
                            text: modelData.label
                            color: pass.labelColor
                        }
                    }
                    Repeater {
                        model: pass.headerFields
                        delegate: QQC2.Label {
                            text: modelData.value
                            color: pass.foregroundColor
                        }
                    }
                }

                // strip image
                Image {
                    source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/strip" : ""
                    sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                    Layout.alignment: Qt.AlignCenter
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
                            color: pass.labelColor
                            text: modelData.label
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                    Repeater {
                        model: pass.primaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: pass.foregroundColor
                            text: modelData.value
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
                            color: pass.labelColor
                            text: modelData.label
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                    Repeater {
                        model: pass.secondaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: pass.foregroundColor
                            text: modelData.value
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
                            color: pass.labelColor
                            text: modelData.label
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                    Repeater {
                        model: pass.auxiliaryFields
                        delegate: QQC2.Label {
                            Layout.fillWidth: true
                            color: pass.foregroundColor
                            text: modelData.value
                            horizontalAlignment: modelData.textAlignment
                        }
                    }
                }
            }

            // barcode
            App.PkPassBarcode { pass: root.pass }

            // footer
            Image {
                source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/footer" : ""
                sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                Layout.alignment: Qt.AlignCenter
            }

            // back fields
            Kirigami.Separator {
                Layout.fillWidth: true
            }
            Repeater {
                model: pass.backFields
                ColumnLayout {
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: pass.labelColor
                        text: modelData.label
                        wrapMode: Text.WordWrap
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: pass.foregroundColor
                        text: modelData.value
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }

    Item {
        id: mask
        anchors.fill: bodyBackground
        Rectangle {
            width: parent.width / 4
            height: width
            radius: width / 2
            color: "black"
            x: parent.width/2 - radius
            y: -radius * 1.5
        }
        visible: false
    }

    Effects.OpacityMask {
        anchors.fill: bodyBackground
        source: bodyBackground
        maskSource: mask
        invert: true
    }
}

