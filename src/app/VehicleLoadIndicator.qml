/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kpublictransport 1.0

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
        text: i18n("FULL")
        color: Kirigami.Theme.negativeTextColor
        visible: maxLoad == Load.Full
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
    }
}
