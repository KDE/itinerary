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
import "." as App

Kirigami.ScrollablePage {
    property alias passId: pkpass.passId
    property alias pass: pkpass.pass
    title: i18n("Boarding Pass")
    Item {
        id: contentItem
        width: parent.width
        implicitHeight: pkpass.implicitHeight

        App.BoardingPass {
            x: (parent.width - implicitWidth) / 2
            id: pkpass
        }
    }

    onBackRequested: pageStack.pop()
}
