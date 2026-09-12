/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.pkpass as KPkPass

/** Common base for all pass variants. */
Item {
    id: root
    /** The pass object to display. */
    property KPkPass.Pass pass: null
    property string passId // TODO can we get rid of this?

    implicitWidth: 332 // as per specification

    /** Default foreground color if the pass doesn't specify a valid one. */
    property color defaultTextColor: palette.text

    /** Double tap on the barcode. */
    signal barcodeDoubleTapped()
}
