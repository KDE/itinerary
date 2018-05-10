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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    id: root
    property var reservation
    property var pass
    property string passId
    property var rangeType

    readonly property double headerFontScale: 1.25

    function showBoardingPass()
    {
        applicationWindow().pageStack.push(pkpassComponent);
    }

    function showTicket()
    {
        applicationWindow().pageStack.push(ticketPageComponent);
    }

    Component {
        id: pkpassComponent
        App.PkPassPage {
            passId: root.passId
            pass: root.pass
        }
    }

    Component {
        id: ticketPageComponent
        App.TicketPage {
            reservation: root.reservation
        }
    }
}
