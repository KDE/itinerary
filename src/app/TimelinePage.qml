/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import "." as App

Kirigami.ScrollablePage {
    title: qsTr("Boarding Passes")
    // context drawer content
    actions {
        contextualActions: [
            Kirigami.Action {
                text: qsTr("Delete Pass")
                iconName: "edit-delete"
                onTriggered: print("TODO")
            }
        ]
    }

    // page content
    ListView {
        model: _timelineModel
        spacing: 5
        delegate: Item {
            height: passElement.implicitHeight
            width: passElement.implicitWidth
            App.BoardingPass {
                x: (parent.ListView.view.width - implicitWidth) / 2
                id: passElement
                pass: model.pass
                passId: model.passId
            }
        }
        section.property: "sectionHeader"
        section.delegate: Rectangle {
            color: Kirigami.Theme.backgroundColor
            implicitHeight: headerItem.implicitHeight
            implicitWidth: ListView.view.width
            Kirigami.BasicListItem {
                id: headerItem
                label: section
                icon: "view-calendar-day"
            }
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
    }
}
