/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

/** Details about a stopover, including:
 *  - notes/service alerts
 *  - occupancy
 *  - vehicle ammentities
 *  - operator/provider
 */
SheetDrawer {
    id: root
    /** The KPublicTransport.Stopover to show here. */
    property KPublicTransport.stopover stopover
    /** Notes/service alerts to show.
     *  This defaults to stopover.notes, overriding
     *  makes sense when e.g. showing a journey section departure.
     */
    property list<string> notes: stopover.notes

    /** @c true if there is any content that can be shown here. */
    readonly property bool hasContent: root.notes.length > 0
        || root.stopover.vehicleLayout.combinedFeatures.length > 0
        || root.stopover.aggregatedOccupancy.length > 0
        || root.stopover.route.line.operatorName !== ""

    contentItem: QQC2.ScrollView {
        id: infoPage
        contentWidth: width - effectiveScrollBarWidth
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        KPublicTransport.StopoverInformationView {
            width: infoPage.width - infoPage.effectiveScrollBarWidth
            stopover: root.stopover
            notes: root.notes
        }
    }
}
