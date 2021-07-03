/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

import org.kde.kirigami 2.17 as Kirigami

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
