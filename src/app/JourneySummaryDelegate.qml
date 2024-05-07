/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

FormCard.AbstractFormDelegate {
    id: root

    required property var journey

    readonly property int sectionWithMaxLoad: {
        var loadMax = Load.Unknown;
        var loadMaxIdx = -1;
        for (var i = 0; root.journey != undefined && i < root.journey.sections.length; ++i) {
            var l = PublicTransport.maximumOccupancy(root.journey.sections[i].loadInformation);
            if (l > loadMax) {
                loadMax = l;
                loadMaxIdx = i;
            }
        }
        return loadMaxIdx;
    }

    contentItem: ColumnLayout {
        RowLayout {
            Repeater {
                model: root.journey.sections
                delegate: TransportIcon {
                    source: modelData.iconName
                    color: PublicTransport.warnAboutSection(modelData) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    isMask: modelData.mode != JourneySection.PublicTransport || (!modelData.route.line.hasLogo && !modelData.route.line.hasModeLogo)
                    size: Kirigami.Units.iconSizes.small
                    Accessible.name: PublicTransport.journeySectionLabel(modelData)
                }
            }
            QQC2.Label {
                text: i18ncp("number of switches to another transport", "One change", "%1 changes", root.journey.numberOfChanges)
                Layout.fillWidth: true
                Accessible.ignored: !parent.visible
            }

            KPublicTransport.OccupancyIndicator {
                occupancy: sectionWithMaxLoad < 0 ? Load.Unknown : PublicTransport.maximumOccupancy(root.journey.sections[sectionWithMaxLoad].loadInformation)
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
            }
        }

        QQC2.Label {
            text: i18n("Click to extend")
        }
    }
}
