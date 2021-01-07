/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.itinerary 1.0
import "." as App

/** Display of a location and navigation actions. */
Item {
    id: root

    /** The timeline delegate controller for the element for which this delegate represents a place. */
    property var controller: null
    /** The place to display and to navigate to. */
    property var place

    /** Indicates that this is the begin (and only the begin) of an element (e.g. departure location of a transit element). */
    property bool isRangeBegin: false
    /** Indicates that this is the end (and only the end) of an element (e.g. arrival location of a transit element). */
    property bool isRangeEnd: false
    /** Indicates that this represents the full range of the element (only valid for non-transit elements). */
    readonly property bool isFullRange: !isRangeBegin && !isRangeEnd

    implicitHeight: (place && (!place.address.isEmpty || place.geo.isValid)) ? Math.max(buttonLayout.implicitHeight, label.implicitHeight) : 0
    implicitWidth: label.width + buttonLayout.width

    Component {
        id: departuresPage
        App.DepartureQueryPage {
            stop: place
            dateTime: controller.effectiveEndTime
        }
    }

    QQC2.Label {
        id: label
        visible: place != undefined && !place.address.isEmpty
        text: place ? Localizer.formatAddress(place.address) : ""
        color: Kirigami.Theme.textColor
        anchors.left: root.left
    }

    Row {
        id: buttonLayout
        anchors.right: root.right
        y: Math.max(0, label.implicitHeight - buttonLayout.implicitHeight)

        QQC2.ToolButton {
            visible: place != undefined && place.geo.isValid
            icon.name: "map-symbolic"
            onClicked: {
                var args = {placeName: place.name};
                if (controller.isLocationChange) {
                    if (isRangeBegin) {
                        args = controller.departureMapArguments();
                    } else if (isRangeEnd) {
                        args = controller.arrivalMapArguments();
                    }
                }
                args.coordinate = Qt.point(place.geo.longitude, place.geo.latitude);
                console.log(JSON.stringify(args));
                applicationWindow().pageStack.push(indoorMapPage, args);
            }
        }

        QQC2.ToolButton {
            visible: place != undefined && (place.geo.isValid || !place.address.isEmpty)
            icon.name: "map-globe"
            onClicked: NavigationController.showOnMap(place)
        }

        // navigate to is offered if:
        // - for departures (begins) of any transit element, unless it's a layover from a previous transit element TODO
        // - for begins of any non-transit element
        // - for the end/arrival of non-public transport transit elements (e.g. car rental drop-offs)
        QQC2.ToolButton {
            visible: NavigationController.canNavigateTo(place) && (!isRangeEnd || (controller.isLocationChange && !controller.isPublicTransport))
            icon.name: "go-next-symbolic"
            onClicked: {
                controller.previousLocation ? NavigationController.navigateTo(controller.previousLocation, place) : NavigationController.navigateTo(place);
            }
        }

        // public transport connections are offered:
        // - at all arrival locations, unless it's a layover to a subsequent transit element TODO
        // -  when leaving non-transit events
        QQC2.ToolButton {
            visible: place != undefined && place.geo.isValid && !isRangeBegin
            icon.name: "view-calendar-day"
            onClicked: {
                applicationWindow().pageStack.push(departuresPage);
            }
        }
    }
}
