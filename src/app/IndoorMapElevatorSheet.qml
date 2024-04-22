/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

Kirigami.Dialog {
    id: elevatorSheet

    property var model

    title: model.title

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        model: elevatorSheet.model

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
