/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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
import QtLocation 5.11 as QtLocation
import QtPositioning 5.11
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    title: i18n("Favorite Locations")

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    QtLocation.Plugin {
        id: mapPlugin
        required.mapping: QtLocation.Plugin.AnyMappingFeatures
        preferred: ["osm"]
    }

    Component.onCompleted: {
        if (combo.count == 0)
            FavoriteLocationModel.appendNewLocation();

    }

   actions.main: Kirigami.Action {
        icon.name: "crosshairs"
        text: i18n("Pick Location")
        onTriggered: {
            var idx = FavoriteLocationModel.index(combo.currentIndex, 0);
            FavoriteLocationModel.setData(idx, map.center.latitude, FavoriteLocationModel.LatitudeRole);
            FavoriteLocationModel.setData(idx, map.center.longitude, FavoriteLocationModel.LongitudeRole);
        }
    }
    actions.contextualActions: [
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
            onTriggered: renameSheet.sheetOpen = true
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
        }
    ]

    Kirigami.OverlaySheet {
        id: renameSheet

        QQC2.Label {
            text: i18n("Rename favorite location")
        }

        footer: ColumnLayout {
            QQC2.TextField {
                id: nameEdit
                Layout.fillWidth: true
                text: combo.currentText
            }
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Rename")
                icon.name: "edit-rename"
                onClicked: {
                    var idx = FavoriteLocationModel.index(combo.currentIndex, 0);
                    FavoriteLocationModel.setData(idx, nameEdit.text, Qt.DisplayRole);
                    renameSheet.sheetOpen = false;
                }
            }
        }
    }


    QQC2.ComboBox {
        id: combo
        anchors { top: parent.top; left: parent.left; right: parent.right; margins: Kirigami.Units.largeSpacing }
        model: FavoriteLocationModel
        textRole: "display"
        onCurrentIndexChanged: {
            var favLoc = delegateModel.items.get(currentIndex)
            map.center = QtPositioning.coordinate(favLoc.model.latitude, favLoc.model.longitude)
        }
    }

    QtLocation.Map {
        id: map
        anchors { top: combo.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; topMargin: Kirigami.Units.largeSpacing }
        plugin: mapPlugin

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
}
