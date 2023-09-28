/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kosmindoormap 1.0

Kirigami.OverlaySheet {
    id: amenitySheet

    property QtObject model
    property QtObject map

    header: Kirigami.Heading {
        text: i18n("Find Amenity")
    }

    ListView {
        model: AmenitySortFilterProxyModel {
            sourceModel: amenitySheet.sheetOpen ? amenitySheet.model : null
            filterString: amenitySearchField.text
        }
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25

        delegate: IndoorMapAmenityDelegate {
            id: item
            mapData: amenitySheet.map.mapData
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
        }
    }

    footer: Kirigami.SearchField {
        id: amenitySearchField
        focus: true
    }

    onSheetOpenChanged: {
        if (sheetOpen)
            amenitySearchField.clear();
    }
}
