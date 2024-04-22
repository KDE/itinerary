/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtLocation as QtLocation
import QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.itinerary

Kirigami.Page {
    id: root
    title: i18n("Favorite Locations")

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    Component.onCompleted: {
        if (combo.count == 0)
            FavoriteLocationModel.appendNewLocation();

    }

    FileDialog {
        id: favoriteGpxExportDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Export Favorite Locations")
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("GPX Files (*.gpx)")]
        onAccepted: FavoriteLocationModel.exportToGpx(selectedFile)
    }
    FileDialog {
        id: favoriteGpxImportDialog
        fileMode: FileDialog.OpenFile
        title: i18n("Import Favorite Locations")
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("GPX Files (*.gpx)")]
        onAccepted: FavoriteLocationModel.importFromGpx(selectedFile)
    }

    actions: [
        Kirigami.Action {
            text: i18n("Add Favorite Location")
            icon.name: "list-add"
            onTriggered: {
                FavoriteLocationModel.appendNewLocation();
                combo.currentIndex = combo.count - 1;
            }
        },
        Kirigami.Action {
            text: i18n("Rename Favorite Location")
            icon.name: "edit-rename"
            onTriggered: renameSheet.open()
        },
        Kirigami.Action {
            text: i18n("Remove Favorite Location")
            icon.name: "edit-delete"
            enabled: combo.count > 1
            onTriggered: {
                var prevIndex = combo.currentIndex;
                FavoriteLocationModel.removeLocation(combo.currentIndex);
                combo.currentIndex = Math.min(prevIndex, combo.count - 1);
            }
        },
        Kirigami.Action {
            text: i18n("Export to GPX")
            icon.name: "export-symbolic"
            onTriggered: favoriteGpxExportDialog.open()
        },
        Kirigami.Action {
            text: i18n("Import from GPX")
            icon.name: "document-import"
            onTriggered: favoriteGpxImportDialog.open()
        }
    ]

    Kirigami.Dialog {
        id: renameSheet

        width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)

        title: i18nc("@title", "Rename favorite location")

        contentItem: QQC2.TextField {
            id: nameEdit
            Layout.fillWidth: true
            text: combo.currentText
        }

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Rename")
                icon.name: "edit-rename"
                onTriggered: {
                    var idx = FavoriteLocationModel.index(combo.currentIndex, 0);
                    FavoriteLocationModel.setData(idx, nameEdit.text, Qt.DisplayRole);
                    renameSheet.close();
                }
            }
        ]
    }


    QQC2.ComboBox {
        id: combo
        anchors { top: parent.top; left: parent.left; right: parent.right; margins: Kirigami.Units.largeSpacing }
        model: FavoriteLocationModel
        textRole: "display"
        onCurrentIndexChanged: {
            var favLoc = delegateModel.items.get(currentIndex)
            map.center = QtPositioning.coordinate(favLoc.model.latitude, favLoc.model.longitude)
            map.zoomLevel = 16;
        }
    }

    MapView {
        id: map
        anchors { top: combo.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; topMargin: Kirigami.Units.largeSpacing }

        QtLocation.MapQuickItem {
            coordinate: map.center
            anchorPoint { x: icon.width / 2; y: icon.height / 2 }
            sourceItem: Kirigami.Icon {
                id: icon
                source: "crosshairs"
                width: height
                height: Kirigami.Units.iconSizes.large
                color: Kirigami.Theme.negativeTextColor
            }
        }
    }

    FloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: Kirigami.Action {
            icon.name: "crosshairs"
            text: i18n("Pick Location")
            onTriggered: {
                var idx = FavoriteLocationModel.index(combo.currentIndex, 0);
                FavoriteLocationModel.setData(idx, map.center.latitude, FavoriteLocationModel.LatitudeRole);
                FavoriteLocationModel.setData(idx, map.center.longitude, FavoriteLocationModel.LongitudeRole);
                applicationWindow().pageStack.goBack();
            }
        }
    }

    footer: null
}
