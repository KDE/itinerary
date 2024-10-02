/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

Item {
    id: root
    property var pass: null
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
            columns: pass.headerFields.length + 2
            Layout.fillWidth: true
            Layout.margins: __margin

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
                    text: modelData.valueDisplayString
                    color: pass.foregroundColor
                }
            }
        }

        ColumnLayout {
            id: bodyLayout
            Layout.margins: __margin
            spacing: __margin

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
                    text: pass.primaryFields[0].label
                    color: pass.labelColor
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    id: primaryValue
                    text: pass.primaryFields[0].valueDisplayString
                    color: pass.foregroundColor
                    font.pointSize: 1.5 * primaryLabel.font.pointSize
                }

                Kirigami.Icon {
                    Layout.rowSpan: 2
                    Layout.alignment: Qt.AlignBottom
                    source: {
                        switch (pass.transitType) {
                            case KPkPass.BoardingPass.Air: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Air)
                            case KPkPass.BoardingPass.Boat: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Ferry)
                            case KPkPass.BoardingPass.Bus: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Bus)
                            case KPkPass.BoardingPass.Train: return KPublicTransport.LineMode.iconName(KPublicTransport.Line.Train)
                        }
                        return "go-next-symbolic";
                    }
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    color: pass.labelColor
                    isMask: true
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.preferredWidth: primaryFieldsLayout.width/2 - Kirigami.Units.iconSizes.smallMedium
                    horizontalAlignment: Qt.AlignRight
                    text: pass.primaryFields[1].label
                    color: pass.labelColor
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    Layout.alignment: Qt.AlignRight
                    text: pass.primaryFields[1].valueDisplayString
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
                        text: modelData.valueDisplayString
                        horizontalAlignment: modelData.textAlignment
                    }
                }
            }

            // footer
            Image {
                source: passId !== "" ? "image://org.kde.pkpass/" + passId + "/footer" : ""
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
            Layout.margins: __margin

            Kirigami.Separator {
                visible: pass.backFields.length > 0
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
                        horizontalAlignment: modelData.textAlignment
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        color: pass.foregroundColor
                        linkColor: color
                        text: Util.textToHtml(modelData.valueDisplayString)
                        textFormat: Util.isRichText(modelData.valueDisplayString) ? Text.StyledText : Text.AutoText
                        wrapMode: Text.WordWrap
                        horizontalAlignment: modelData.textAlignment
                        onLinkActivated: Qt.openUrlExternally(link)
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
            radius: __margin
            color: pass.backgroundColor
            Layout.fillWidth: true
            implicitHeight: headerLayout.implicitHeight + 2 * __margin
        }

        Rectangle {
            id: bodyBackground
            radius: __margin
            color: pass.backgroundColor
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
