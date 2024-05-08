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

Kirigami.Dialog {
    id: elementDetailsSheet

    property var model
    property var mapData

    title: elementDetailsSheet.model.name + (elementDetailsSheet.model.category.length  > 0 ? (" - " + elementDetailsSheet.model.category) : "")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        id: contentView
        model: elementDetailsSheet.model
        clip: true
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25

        Component {
            id: infoStringDelegate
            RowLayout {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
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
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
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
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                text: row ? Localizer.formatAddress(row.value) : ""
            }
        }

        Component {
            id: infoOpeningHoursDelegate
            OSMElementInformationDialogOpeningHoursDelegate {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                regionCode: elementDetailsSheet.mapData.regionCode
                timeZoneId: elementDetailsSheet.mapData.timeZone
                latitude: elementDetailsSheet.mapData.center.y
                longitude: elementDetailsSheet.mapData.center.x
                openingHours: row.value
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

    customFooterActions: [
        Kirigami.Action {
            visible: Settings.osmContributorMode
            icon.name: "document-edit"
            text: i18n("Edit with iD")
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.ID)
        },
        Kirigami.Action {
            visible: Settings.osmContributorMode && EditorController.hasEditor(Editor.JOSM)
            icon.name: "org.openstreetmap.josm"
            text: i18n("Edit with JOSM")
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.JOSM)
        },
        Kirigami.Action {
            visible: Settings.osmContributorMode && EditorController.hasEditor(Editor.Vespucci)
            icon.name: "document-edit"
            text: i18n("Edit with Vespucci")
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.Vespucci)
        }
    ]

    onClosed: elementDetailsSheet.model.clear()
}
