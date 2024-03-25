/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport

import org.kde.kosmindoormap // for the icon assets

Kirigami.Icon {
    id: root

    property var feature

    Layout.preferredHeight: Kirigami.Units.iconSizes.small
    Layout.preferredWidth: Kirigami.Units.iconSizes.small

    source: {
        switch (root.feature.type) {
            case KPublicTransport.Feature.AirConditioning:
                return "temperature-cold";
            case KPublicTransport.Feature.Restaurant:
                return "qrc:///images/foodestablishment.svg"
            case KPublicTransport.Feature.ToddlerArea:
                return "qrc:///images/toddler.svg";
            case KPublicTransport.Feature.FamilyArea:
                return "qrc:///images/family.svg";
            case KPublicTransport.Feature.WheelchairAccessible:
                return "qrc:///images/wheelchair_accessible.svg";
            case KPublicTransport.Feature.SilentArea:
                return "player-volume-muted";
            case KPublicTransport.Feature.BikeStorage:
                return "qrc:///images/bike.svg";
            case KPublicTransport.Feature.Toilet:
                return "qrc:///org.kde.kosmindoormap/assets/icons/toilets.svg";
            case KPublicTransport.Feature.WheelchairAccessibleToilet:
                return "qrc:///images/wheelchair.svg";
            case KPublicTransport.Feature.InformationPoint:
                return "qrc:///org.kde.kosmindoormap/assets/icons/information.svg";
            case KPublicTransport.Feature.WiFi:
                return "network-wireless-symbolic";
            case KPublicTransport.Feature.Other:
            default:
                return "documentinfo";
        }
    }

    Kirigami.Icon {
        id: emblem
        anchors {
            right: root.right
            bottom: root.bottom
        }
        width: root.width / 2
        height: root.height / 2

        visible: emblem.source !== ""
        source: {
            if (root.feature.disruptionEffect === KPublicTransport.Disruption.NoService)
                return s + "emblem-warning";

            switch (root.feature.availability) {
                case KPublicTransport.Feature.Unknown:
                    return "emblem-question";
                case KPublicTransport.Feature.Unavailable:
                    return "emblem-error";
                case KPublicTransport.Feature.Limited:
                    return "emblem-information";
                case KPublicTransport.Feature.Conditional:
                    return "emblem-important";
                case KPublicTransport.Feature.Available:
                default:
                    break;
            }
        }
    }

    HoverHandler {
        id: hoverHandler
    }
    QQC2.ToolTip.visible: hoverHandler.hovered && QQC2.ToolTip.text !== ""
    QQC2.ToolTip.text: {
        let s = root.feature.name;
        if (s === "") {
            switch (root.feature.type) {
                case KPublicTransport.Feature.AirConditioning:
                    s = i18nc("train coach feature", "Air conditioning");
                    break;
                case KPublicTransport.Feature.Restaurant:
                    s = i18nc("train coach feature", "Bistro or restaurant");
                    break;
                case KPublicTransport.Feature.ToddlerArea:
                    s = i18nc("train coach feature", "Toddler area");
                    break;
                case KPublicTransport.Feature.FamilyArea:
                    s = i18nc("train coach feature", "Family area");
                    break;
                case KPublicTransport.Feature.WheelchairAccessible:
                    s = i18nc("train coach feature", "Wheelchar accessible");
                    break;
                case KPublicTransport.Feature.SilentArea:
                    s = i18nc("train coach feature", "Quiet area");
                    break;
                case KPublicTransport.Feature.BikeStorage:
                    s = i18nc("train coach feature", "Bike storage");
                    break;
                case KPublicTransport.Feature.Toilet:
                    s = i18nc("train coach feature", "Toilet");
                    break;
                case KPublicTransport.Feature.WheelchairAccessibleToilet:
                    s = i18nc("train coach feature", "Wheelchar accessible toilet");
                    break;
                case KPublicTransport.Feature.InformationPoint:
                    s = i18nc("train coach feature", "Information point");
                    break;
                case KPublicTransport.Feature.WiFi:
                    s = i18nc("train coach feature", "Wi-Fi");
                    break;
                case KPublicTransport.Feature.Other:
                default:
                    break;
            }
        }

        if (root.feature.availability === KPublicTransport.Feature.Unavailable) {
            s = i18n("%1 (not available)", s)
        }

        if (root.feature.description !== "") {
            s += '\n' + root.feature.description;
            s = s.trimmed();
        }

        return s;
    }
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
}
