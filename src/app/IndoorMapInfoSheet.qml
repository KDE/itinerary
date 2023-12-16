/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap
import org.kde.osm.editorcontroller
import org.kde.itinerary

Kirigami.OverlaySheet {
    id: elementDetailsSheet
    property var model
    property var mapData

    header: Column {
        Kirigami.Heading {
            text: elementDetailsSheet.model.name
            width: parent.width
            wrapMode: Text.WordWrap
        }
        Kirigami.Heading {
            text: elementDetailsSheet.model.category
            level: 4
            visible: text != ""
            width: parent.width
            wrapMode: Text.WordWrap
        }
    }

    ListView {
        id: contentView
        model: elementDetailsSheet.model
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25

        Component {
            id: infoStringDelegate
            RowLayout {
                QQC2.Label {
                    visible: row && row.keyLabel != ""
                    text: row ? row.keyLabel + ":" : ""
                    color: (row && row.category == OSMElementInformationModel.DebugCategory) ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                    Layout.alignment: Qt.AlignTop
                }
                QQC2.Label {
                    text: row ? row.value : ""
                    color: (row && row.category == OSMElementInformationModel.DebugCategory) ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Component {
            id: infoLinkDelegate
            RowLayout {
                QQC2.Label {
                    visible: row && row.keyLabel != ""
                    text: row ? row.keyLabel + ":" : ""
                    color: (row && row.category == OSMElementInformationModel.DebugCategory) ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                    Layout.alignment: Qt.AlignTop
                }
                QQC2.Label {
                    text: row ? "<a href=\"" + row.url + "\">" + row.value + "</a>" : ""
                    color: (row && row.category == OSMElementInformationModel.DebugCategory) ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                    onLinkActivated: Qt.openUrlExternally(link)
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Component {
            id: infoAddressDelegate
            QQC2.Label {
                text: row ? Localizer.formatAddress(row.value) : ""
            }
        }

        Component {
            id: infoOpeningHoursDelegate
            IndoorMapInfoSheetOpeningHoursDelegate {
                mapData: elementDetailsSheet.mapData
                model: row
            }
        }

        section.property: "categoryLabel"
        section.delegate: Kirigami.Heading {
            x: Kirigami.Units.largeSpacing
            level: 4
            text: section
            color: section == "Debug" ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
            height: implicitHeight + Kirigami.Units.largeSpacing
            verticalAlignment: Qt.AlignBottom
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.InlineLabels

        delegate: Loader {
            property var row: model
            x: Kirigami.Units.largeSpacing
            width: ListView.view.width - 2 * x
            sourceComponent: {
                switch (row.type) {
                    case OSMElementInformationModel.Link:
                        return infoLinkDelegate;
                    case OSMElementInformationModel.PostalAddress:
                        return infoAddressDelegate;
                    case OSMElementInformationModel.OpeningHoursType:
                        return infoOpeningHoursDelegate;
                    case OSMElementInformationModel.String:
                    default:
                        return infoStringDelegate;
                }
            }
        }
    }

    property var footerLoader: Loader {
        active: Settings.osmContributorMode
        sourceComponent: RowLayout {
            Item { Layout.fillWidth: true }
            QQC2.Button {
                icon.name: "document-edit"
                text: i18n("Edit with iD")
                onClicked: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.ID)
            }
            QQC2.Button {
                icon.name: "org.openstreetmap.josm"
                text: i18n("Edit with JOSM")
                visible: EditorController.hasEditor(Editor.JOSM)
                onClicked: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.JOSM)
            }
            QQC2.Button {
                icon.name: "document-edit"
                text: i18n("Edit with Vespucci")
                visible: EditorController.hasEditor(Editor.Vespucci)
                onClicked: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.Vespucci)
            }
        }
    }
    footer: Settings.osmContributorMode ? elementDetailsSheet.footerLoader.item : null

    onClosed: elementDetailsSheet.model.clear()
}
