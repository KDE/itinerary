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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.pkpass 1.0 as KPkPass
import "." as App

Item {
    id: root
    property var pass: null
    property string passId
    //implicitHeight: frontLayout.implicitHeight
    implicitWidth: Math.max(frontLayout.implicitWidth, 332)

    ColumnLayout {
        id: frontLayout
        spacing: 0
        anchors.fill: parent
        // HACK to break binding loop on implicitHeight
        onImplicitHeightChanged: root.implicitHeight = implicitHeight

        Rectangle {
            id: headerBackground
            radius: 10
            color: pass.backgroundColor
            Layout.fillWidth: true
            implicitWidth: headerLayout.implicitWidth + 2 * headerLayout.anchors.margins
            implicitHeight: headerLayout.implicitHeight + 2 * headerLayout.anchors.margins

            GridLayout {
                id: headerLayout
                rows: 2
                columns: pass.headerFields.length + 2
                anchors.fill: parent
                anchors.margins: 10

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
        }

        Rectangle {
            id: bodyBackground
            radius: 10
            color: pass.backgroundColor
            Layout.fillWidth: true
            implicitHeight: bodyLayout.implicitHeight + 2 * bodyLayout.anchors.margins
            implicitWidth: bodyLayout.implicitWidth + 2 * bodyLayout.anchors.margins

            ColumnLayout {
                id: bodyLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

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
                        Layout.preferredWidth: 1
                        text: pass.primaryFields[0].label
                        color: pass.labelColor
                    }
                    QQC2.Label {
                        id: primaryValue
                        text: pass.primaryFields[0].value
                        color: pass.foregroundColor
                        font.pointSize: 1.5 * primaryLabel.font.pointSize
                    }

                    Kirigami.Icon {
                        Layout.rowSpan: 2
                        Layout.alignment: Qt.AlignBottom
                        // TODO: check transit type and use appropriate icons
                        source: "qrc:///images/flight.svg"
                        width: Kirigami.Units.iconSizes.smallMedium
                        height: width
                        color: pass.labelColor
                        isMask: true
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        horizontalAlignment: Qt.AlignRight
                        text: pass.primaryFields[1].label
                        color: pass.labelColor
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: pass.primaryFields[1].value
                        color: pass.foregroundColor
                        font.pointSize: primaryValue.font.pointSize
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

                // footer
                Image {
                    source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/footer" : ""
                    sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
                    Layout.alignment: Qt.AlignCenter
                }

                // barcode
                App.PkPassBarcode { pass: root.pass }

                // TODO back fields
            }
        }
    }
}
