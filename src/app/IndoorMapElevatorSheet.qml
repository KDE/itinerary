/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

Kirigami.OverlaySheet {
    id: elevatorSheet
    property var model

    header: Kirigami.Heading {
        text: model.title
    }

    ListView {
        model: elevatorSheet.model
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10

        delegate: QQC2.ItemDelegate {
            highlighted: false
            width: ListView.view.width
            contentItem: Kirigami.TitleSubtitle {
                title: model.display
                font.bold: model.isCurrentFloor
            }
            onClicked: {
                elevatorSheet.close();
                map.view.floorLevel = model.floorLevel;
            }
        }
    }
}
