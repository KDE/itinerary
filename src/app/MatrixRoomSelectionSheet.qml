// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kitemmodels 1.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.itinerary 1.0
import "." as App

// TODO sort room list in activity order
// TODO remember last selected room
Kirigami.OverlaySheet {
    id: sheet
    signal roomSelected(var room)

    header: Kirigami.Heading { text: i18n("Share Location") }
    ListView {
        model: KSortFilterProxyModel {
            // don't set the model right away, that seems to result in all delegates being instantiated before the sheet is even shown
            sourceModel: sheet.sheetOpen ? MatrixController.roomsModel : null
            sortRole: MatrixRoomsModel.DisplayNameRole
            isSortLocaleAware: true
            filterCaseSensitivity: Qt.CaseInsensitive
            filterString: searchField.text
        }

        delegate: Kirigami.AbstractListItem {
            required property string displayName
            required property var avatar
            required property var avatarImage
            required property string topic
            required property string id
            required property int index

            GridLayout {
                rows: 2
                columns: 3
                Kirigami.Avatar {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large
                    Layout.rowSpan: 2
                    Layout.row: 0
                    Layout.column: 0
                    name: displayName
                    visible: !avatar
                }
                Kirigami.Icon { // FIXME hack as Avatar cannot consume a QImage, and we don't have the mxc image provider here (yet)
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large
                    Layout.rowSpan: 2
                    Layout.row: 0
                    Layout.column: 1
                    source: avatarImage
                    visible: avatar
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.row: 0
                    Layout.column: 2
                    text: displayName
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.row: 1
                    Layout.column: 2
                    text: topic
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            onClicked: {
                console.log(id);
                sheet.roomSelected({ id: id, displayName: displayName });
                sheet.close();
            }
        }
    }

    footer: Kirigami.SearchField {
        id: searchField
        focus: true
    }

    onSheetOpenChanged: {
        if (sheetOpen)
            searchField.clear();
    }
}
