/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kosmindoormap 1.0

Kirigami.OverlaySheet {
    id: elevatorSheet
    property var model

    header: Kirigami.Heading {
        text: model.title
    }

    ListView {
        model: elevatorSheet.model
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10

        delegate: Kirigami.BasicListItem {
            highlighted: false
            label: model.display
            bold: model.isCurrentFloor
            onClicked: {
                elevatorSheet.close();
                map.view.floorLevel = model.floorLevel;
            }
        }
    }
}
