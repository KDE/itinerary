/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary


/** Map view showing a single journey section. */
MapView {
    id: root

    /** Journey section to show. */
    property KPublicTransport.journeySection journeySection

    /** Effective line color used for the overlays. */
    readonly property color lineColor: line.line.color

    /** Emitted when a specific stopover of the journey section was clicked. */
    signal stopoverClicked(elem: var)

    /** Position and zoom the map for the entire journey to fit into the view. */
    function centerOnJourney() {
        if (root.journeySection.mode === KPublicTransport.JourneySection.Invalid)
            return;
        const bbox = KPublicTransport.MapUtils.boundingBox(root.journeySection);
        root.center = KPublicTransport.MapUtils.center(bbox);
        root.zoomLevel = KPublicTransport.MapUtils.zoomLevel(bbox, root.width, root.height);
    }

    QtLocation.MapPolyline {
        id: line
        line.width: 10
        // hardcoded Breeze black, can't use Kirigami theme colors as we need contrast to OSM tiles here, also in dark mode
        line.color: root.journeySection.route.line.hasColor ? root.journeySection.route.line.color : "#232629"
        path: KPublicTransport.MapUtils.polyline(root.journeySection);
    }

    // departure stop
    MapCircle {
        coordinate {
            latitude: root.journeySection.departure.stopPoint.latitude
            longitude: root.journeySection.departure.stopPoint.longitude
        }
        color: line.line.color
        size: 15
        onClicked: root.stopoverClicked({ stopover: root.journeySection.departure, isArrival: false, isDeparture: true })
    }

    // intermediate stops
    QtLocation.MapItemView {
        model: root.journeySection.intermediateStops
        delegate: MapCircle {
            id: delegateRoot
            required property KPublicTransport.stopover modelData
            coordinate {
                latitude: modelData.stopPoint.latitude
                longitude: modelData.stopPoint.longitude
            }
            size: 6
            borderWidth: 1
            color: line.line.color
            textColor: Qt.alpha(modelData.disruptionEffect === KPublicTransport.Disruption.NoService ? Kirigami.Theme.negativeTextColor : "#eff0f1", 0.5)
            onClicked: root.stopoverClicked({ stopover: modelData, isArrival: false, isDeparture: false })
        }
    }

    // arrival stop
    MapCircle {
        coordinate {
            latitude: root.journeySection.arrival.stopPoint.latitude
            longitude: root.journeySection.arrival.stopPoint.longitude
        }
        color: line.line.color
        size: 15
        onClicked: root.stopoverClicked({ stopover: root.journeySection.arrival, isArrival: true, isDeparture: false })
    }

    onWidthChanged: centerOnJourney()
    onHeightChanged: centerOnJourney()
    onJourneySectionChanged: centerOnJourney()
    Component.onCompleted: centerOnJourney()
}
