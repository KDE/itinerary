/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport

Item {
    id: root

    property var loadInformation
    implicitWidth: loadIcon.visible ? loadIcon.width : fullMarker.visible ? fullMarker.implicitWidth : 0
    implicitHeight: childrenRect.height

    // TODO specify filter criteria like class

    /** Display occupancy value, default to the maximum value in @p loadInformation. */
    property var load: {
        var load = Load.Unknown;
        for (var i = 0; loadInformation != undefined && i < loadInformation.length; ++i) {
            load = Math.max(load, loadInformation[i].load);
        }
        return load;
    }

    QQC2.Label {
        id: fullMarker
        text: i18nc("vehicle load", "FULL")
        color: Kirigami.Theme.negativeTextColor
        visible: root.load == Load.Full
        Accessible.ignored: !visible
    }

    Kirigami.Icon {
        id: loadIcon
        visible: root.load != Load.Full && root.load != Load.Unknown
        source: "qrc:///images/seat.svg"
        isMask: true
        height: Kirigami.Units.iconSizes.small
        width: loadIcon.height
        color: {
            switch (root.load) {
                case Load.Low: return Kirigami.Theme.positiveTextColor;
                case Load.Medium: return Kirigami.Theme.neutralTextColor;
                case Load.High: return Kirigami.Theme.negativeTextColor;
            }
            return Kirigami.Theme.textColor;
        }
        Accessible.name: {
            switch (root.load) {
                case Load.Low: return i18nc("vehicle load", "Low occupancy");
                case Load.Medium: return i18nc("vehicle load", "Medium occupancy")
                case Load.High: return i18nc("vehicle load", "High occupancy")
            }
            return null;
        }
        Accessible.ignored: !loadIcon.visible

        HoverHandler { id: hoverHandler }
        QQC2.ToolTip.visible: hoverHandler.hovered && Accessible.name !== ""
        // TODO show per-class occupancy when available
        QQC2.ToolTip.text: Accessible.name
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
