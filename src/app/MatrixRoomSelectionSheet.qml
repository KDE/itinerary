// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kitemmodels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as AddonComponents
import org.kde.itinerary
import "." as App

// TODO sort room list in activity order
// TODO remember last selected room
Kirigami.OverlaySheet {
    id: sheet
    signal roomSelected(var room)

    header: Kirigami.Heading { text: i18n("Share Location") }
    ListView {
        clip: true
        model: KSortFilterProxyModel {
            // don't set the model right away, that seems to result in all delegates being instantiated before the sheet is even shown
            sourceModel: sheet.visible ? MatrixController.roomsModel : null
            filterCaseSensitivity: Qt.CaseInsensitive
            filterString: searchField.text
        }

        section {
            property: "category"
            delegate: Kirigami.ListSectionHeader {
                text: {
                    switch (section * 1) {
                        case MatrixRoomsModel.InvitedRoom: return i18nc("matrix room type", "Invited");
                        case MatrixRoomsModel.FavoriteRoom: return i18nc("matrix room type", "Favorite Rooms");
                        case MatrixRoomsModel.DirectChatRoom: return i18nc("matrix room type", "Direct Messages");
                        case MatrixRoomsModel.RegularRoom: return i18nc("matrix room type", "Rooms");
                        case MatrixRoomsModel.LowPriorityRoom: return i18nc("matrix room type", "Low Priority");
                        case MatrixRoomsModel.Space: return i18nc("matrix room type", "Spaces");
                    }
                }
                width: ListView.view.width
            }
        }

        delegate: QQC2.ItemDelegate {
            required property string displayName
            required property var avatar
            required property var avatarImage
            required property string topic
            required property string id
            required property int index

            width: ListView.view.width
            contentItem: GridLayout {
                rows: 2
                columns: 3
                AddonComponents.Avatar {
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
                    elide: Text.ElideRight
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

    onOpened: searchField.clear()
}
