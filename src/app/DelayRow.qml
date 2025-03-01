// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport

RowLayout {
    id: root

    spacing: 0

    required property KPublicTransport.stopover stopover
    required property int delay
    required property string originalTime

    Kirigami.Heading {
        level: 5
        text: if (root.stopover.disruptionEffect === KPublicTransport.Disruption.NoService) {
            return i18nc("a train/bus journey canceled by its operator", "Canceled") + ' - ';
        } else if (oldTime.visible) {
            return i18nc("duration of the delay", "Delayed %1", Localizer.formatDuration(root.delay * 60)) + ' - ';
        } else {
            return i18nc("@info", "On time");
        }
        color: oldTime.visible ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
        font.weight: Font.DemiBold
    }

    QQC2.Label {
        id: oldTime

        opacity: 0.8
        text: root.originalTime
        font.strikeout: true
        visible: root.delay * 60 > 1 || root.stopover.disruptionEffect === KPublicTransport.Disruption.NoService
    }
}
