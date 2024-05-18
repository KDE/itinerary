/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

Kirigami.Dialog {
    id: amenitySheet

    property QtObject model
    property QtObject map

    title: i18nc("@title", "Find Amenity")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        model: AmenitySortFilterProxyModel {
            id: proxyModel
            sourceModel: amenitySheet.visible ? amenitySheet.model : null
            filterString: amenitySearchField.text
        }

        header: QQC2.Control {
            width: parent.width
            contentItem: Kirigami.SearchField {
                id: searchField
                onTextChanged: proxyModel.filterString = text

                Connections {
                    target: sheet
                    function onVisibleChanged() {
                        searchField.text = "";
                    }
                }
                focus: true
            }
        }

        clip:true

        delegate: AmenityListDelegate {
            id: item
            required property QtObject model
            onClicked: {
                amenitySheet.map.view.floorLevel = item.model.level
                amenitySheet.map.view.setZoomLevel(21, Qt.point(amenitySheet.map.width / 2.0, amenitySheet.map.height/ 2.0));
                amenitySheet.map.view.centerOnGeoCoordinate(item.model.coordinate);
                amenitySheet.close();
            }
        }

        section.property: "groupName"
        section.delegate: Kirigami.ListSectionHeader {
            label: section
            width: ListView.view.width
        }
    }
}
