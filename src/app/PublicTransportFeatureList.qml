/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KirigamiDelegates
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport

Repeater {
    delegate: RowLayout {
        Layout.leftMargin: Kirigami.Units.largeSpacing
        KPublicTransport.FeatureIcon {
            id: featureIcon
            feature: modelData
            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
        }
        KirigamiDelegates.TitleSubtitle {
            Layout.fillWidth: true
            enabled: modelData.availability !== KPublicTransport.Feature.Unavailable
            title: modelData.displayName
            subtitle: {
                if (modelData.description !== "")
                    return modelData.description;
                if (modelData.availability === KPublicTransport.Feature.Unknown)
                    return i18n("Availability unknown")
                if (modelData.availability === KPublicTransport.Feature.Unavailable)
                    return i18n("Not available")
                if (modelData.disruptionEffect === KPublicTransport.Disruption.NoService)
                    return i18n("Currently not available")
                if (modelData.quantity > 0)
                    return i18np("One space", "%1 spaces", modelData.quantity);
                return "";
            }
        }
    }
}
