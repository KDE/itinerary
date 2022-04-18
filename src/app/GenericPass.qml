/*
    SPDX-FileCopyrightText: 2018-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.pkpass 1.0 as KPkPass
import org.kde.itinerary 1.0
import "." as App

Rectangle {
    id: root
    property var pass: null
    property string passId
    implicitHeight: bodyBackground.implicitHeight
    implicitWidth: 332 //Math.max(topLayout.implicitWidth, 332)

    color: Util.isValidColor(pass.backgroundColor) ? pass.backgroundColor : Kirigami.Theme.backgroundColor
    radius: 10

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    ColumnLayout {
        id: topLayout
        spacing: 10
        anchors.fill: parent
        anchors.margins: 6
        // HACK to break binding loop on implicitHeight
        onImplicitHeightChanged: root.implicitHeight = implicitHeight + 2 * topLayout.anchors.margins

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
                color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : Kirigami.Theme.textColor
            }

            Repeater {
                model: pass.headerFields
                delegate: QQC2.Label {
                    text: modelData.label
                    color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : Kirigami.Theme.textColor
                }
            }
            Repeater {
                model: pass.headerFields
                delegate: QQC2.Label {
                    text: modelData.valueDisplayString
                    color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : Kirigami.Theme.textColor
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
            columns: pass.primaryFields.length
            Layout.fillWidth: true

            Repeater {
                model: pass.primaryFields
                delegate: QQC2.Label {
                    Layout.fillWidth: true
                    color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : Kirigami.Theme.textColor
                    text: modelData.label
                    horizontalAlignment: modelData.textAlignment
                }
            }
            Repeater {
                model: pass.primaryFields
                delegate: QQC2.Label {
                    Layout.fillWidth: true
                    color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : Kirigami.Theme.textColor
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
                    color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : Kirigami.Theme.textColor
                    text: modelData.label
                    horizontalAlignment: modelData.textAlignment
                }
            }
            Repeater {
                model: pass.secondaryFields
                delegate: QQC2.Label {
                    Layout.fillWidth: true
                    color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : Kirigami.Theme.textColor
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
                    color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : Kirigami.Theme.textColor
                    text: modelData.label
                    horizontalAlignment: modelData.textAlignment
                }
            }
            Repeater {
                model: pass.auxiliaryFields
                delegate: QQC2.Label {
                    Layout.fillWidth: true
                    color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : Kirigami.Theme.textColor
                    text: modelData.valueDisplayString
                    horizontalAlignment: modelData.textAlignment
                }
            }
        }

        // barcode
        App.PkPassBarcode {
            pass: root.pass
            TapHandler {
                onDoubleTapped: scanModeToggled()
            }
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
                    color: Util.isValidColor(pass.labelColor) ?  pass.labelColor : Kirigami.Theme.textColor
                    text: modelData.label
                    wrapMode: Text.WordWrap
                    horizontalAlignment: modelData.textAlignment
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    color: Util.isValidColor(pass.foregroundColor) ?  pass.foregroundColor : Kirigami.Theme.textColor
                    text: Util.textToHtml(modelData.valueDisplayString)
                    textFormat: Util.isRichText(modelData.valueDisplayString) ? Text.RichText : Text.AutoText
                    wrapMode: Text.WordWrap
                    horizontalAlignment: modelData.textAlignment
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }
    }
}
