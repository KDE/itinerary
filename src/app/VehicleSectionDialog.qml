/*
    SPDX-FileCopyrightText: ⓒ 2019 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.components as Addons
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport

/** Information dialog for a single train coach. */
Addons.ConvergentContextMenu {
    id: root

    /** The train coach to display. */
    property KPublicTransport.vehicleSection coach

    displayMode: Kirigami.Settings.isMobile ? Addons.ConvergentContextMenu.BottomDrawer : Addons.ConvergentContextMenu.Dialog

    // TODO show platform section as well?
    headerContentItem: ColumnLayout {
        Kirigami.Heading {
            text: i18nc("train coach", "Coach %1", root.coach.name)
            elide: Qt.ElideRight

            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
        }

        QQC2.Label {
            id: subtitle

            text: {
                const s = root.coach.typeName;
                return s !== "" ? s : root.coach.classesName;
            }
            visible: subtitle.text !== ""

            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
        }
    }


    Kirigami.Action {
        displayComponent: FormCard.AbstractFormDelegate {
            background: null
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.mediumSpacing
                Repeater {
                    model: root.coach.sectionFeatures
                    delegate: KPublicTransport.FeatureDelegate {
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        required property KPublicTransport.feature modelData
                        feature: modelData
                    }
                }

                KPublicTransport.OccupancyDelegate {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    occupancy: root.coach.load
                    visible: occupancy != KPublicTransport.Load.Unknown
                }
            }
        }
    }
}
