/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport

KPublicTransport.OccupancyIndicator {
    id: root

    property var loadInformation
    Layout.preferredHeight: Kirigami.Units.iconSizes.small
    Layout.preferredWidth: Kirigami.Units.iconSizes.small

    // TODO specify filter criteria like class

    /** Display occupancy value, default to the maximum value in @p loadInformation. */
    property var load: {
        var load = Load.Unknown;
        for (var i = 0; loadInformation != undefined && i < loadInformation.length; ++i) {
            load = Math.max(load, loadInformation[i].load);
        }
        return load;
    }

    occupancy: load
}
