/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport

Item {
    property var loadInformation
    implicitWidth: loadIcon.visible ? loadIcon.width : fullMarker.visible ? fullMarker.implicitWidth : 0
    implicitHeight: childrenRect.height

    // TODO specify filter criteria like class

    readonly property var maxLoad: {
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
        visible: maxLoad == Load.Full
        Accessible.ignored: !visible
    }

    Kirigami.Icon {
        id: loadIcon
        visible: maxLoad != Load.Full && maxLoad != Load.Unknown
        source: "qrc:///images/seat.svg"
        isMask: true
        height: Kirigami.Units.iconSizes.small
        width: height
        color: {
            switch (maxLoad) {
                case Load.Low: return Kirigami.Theme.positiveTextColor;
                case Load.Medium: return Kirigami.Theme.neutralTextColor;
                case Load.High: return Kirigami.Theme.negativeTextColor;
            }
            return Kirigami.Theme.textColor;
        }
        Accessible.name: {
            switch (maxLoad) {
                case Load.Low: return i18nc("vehicle load", "Low");
                case Load.Medium: return i18nc("vehicle load", "Medium")
                case Load.High: return i18nc("vehicle load", "High")
            }
            return null;
        }
        Accessible.ignored: !visible
    }
}
