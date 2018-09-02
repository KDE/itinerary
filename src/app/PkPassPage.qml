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
import org.kde.kirigami 2.0 as Kirigami
import org.kde.pkpass 1.0 as KPkPass
import "." as App

Kirigami.ScrollablePage {
    id: root
    property string passId
    property var pass
    title: {
        switch (pass.type) {
            case KPkPass.Pass.BoardingPass: return i18n("Boarding Pass");
            case KPkPass.Pass.EventTicket: return i18n("Event Ticket");
        }
    }

    Component {
        id: boardingPass
        App.BoardingPass {
            passId: root.passId
            pass: root.pass
        }
    }

    Component {
        id: eventTicket
        App.EventTicket {
            passId: root.passId
            pass: root.pass
        }
    }

    Item {
        id: contentItem
        width: parent.width
        implicitHeight: loader.item.implicitHeight

        Loader {
            id: loader
            x: (parent.width - implicitWidth) / 2
            sourceComponent: {
                switch (root.pass.type) {
                    case KPkPass.Pass.BoardingPass: return boardingPass;
                    case KPkPass.Pass.EventTicket: return eventTicket;
                }
            }
        }
    }

    onBackRequested: pageStack.pop()
}
