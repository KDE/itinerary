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
// TODO room icon/avatar
Kirigami.OverlaySheet {
    id: sheet
    signal roomSelected(var room)

    header: Kirigami.Heading { text: i18n("Share Location") }
    ListView {
        model: KSortFilterProxyModel {
            sourceModel: MatrixRoomsModel {
                connection: MatrixManager.connection
            }
            sortRole: MatrixRoomsModel.DisplayNameRole
        }

        delegate: Kirigami.BasicListItem {
            label: model.displayName
            onClicked: {
                console.log(model.id);
                sheet.roomSelected({ id: model.id, displayName: model.displayName });
                sheet.close();
            }
        }
    }
}
