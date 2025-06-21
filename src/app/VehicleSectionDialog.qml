/*
    SPDX-FileCopyrightText: ⓒ 2019 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport

/** Information dialog for a single train coach. */
SheetDrawer {
    id: root

    /** The train coach to display. */
    property KPublicTransport.vehicleSection coach

    headerItem: Component {
        // TODO show platform section as well?
        ColumnLayout {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Kirigami.Heading {
                text: i18nc("train coach", "Coach %1", root.coach.name)
                Layout.fillWidth: true
                elide: Qt.ElideRight
            }
            QQC2.Label {
                id: subtitle
                Layout.fillWidth: true
                text: {
                    const s = root.coach.typeName;
                    return s !== "" ? s : root.coach.classesName;
                }
                visible: subtitle.text !== ""
            }
        }
    }

    contentItem: Component {
        ColumnLayout {
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
