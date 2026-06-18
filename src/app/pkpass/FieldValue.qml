// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.pkpass as KPkPass

/*! Value part of as pass field. */
Controls.Label {
    /*! The field the value belongs to. */
    property KPkPass.field field

    Layout.fillWidth: true
    text: field.valueDisplayString
    horizontalAlignment: field.textAlignment
}
