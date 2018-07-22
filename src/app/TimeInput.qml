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

import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

import org.kde.kirigami 2.4 as Kirigami

// temporary time input until we have that in Kirigami and David's prototype in
// plasma-desktop depends on Plasma which we don't have on Android
RowLayout {
    id: layout
    property date value

    SpinBox {
        from: 0
        to: 23
        editable: true
        wrap: true
        value: layout.value.getHours()
        onValueChanged: {
            if (isNaN(layout.value.getTime()))
                return;
            var dt = layout.value;
            dt.setHours(value);
            layout.value = dt;
        }
    }
    Label { text: ':' }
    SpinBox {
        from: 0
        to: 59
        editable: true
        wrap: true
        value: layout.value.getMinutes()
        onValueChanged: {
            if (isNaN(layout.value.getTime()))
                return;
            var dt = layout.value;
            dt.setMinutes(value);
            layout.value = dt;
        }
    }
}
