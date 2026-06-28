/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.pkpass as KPkPass

/*! Background for regular generic passes. */
Rectangle {
    id: root

    /*! The pass these fields are from. */
    property KPkPass.Pass pass: null

    /*! TODO can we get this from pass directly? */
    property string passId

    color: root.pass.hasBackgroundColor ? root.pass.backgroundColor : Kirigami.Theme.backgroundColor
    radius: 10

    // TODO opacity mask to round the corners
    Image {
        id: backgroundImage
        source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/background" : ""
        fillMode: backgroundImage.implicitHeight < parent.implicitHeight ? Image.TileVertically : Image.PreserveAspectCrop
        verticalAlignment: Image.AlignTop
        horizontalAlignment: Image.AlignHCenter
        x: -(width - root.width) / 2
        y: 0
        height: root.height
        width: root.width
    }
}
