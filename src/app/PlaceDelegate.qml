/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.itinerary 1.0
import "." as App

/** Display of a location and navigation actions. */
ColumnLayout {
    id: root

    /** The timeline delegate controller for the element for which this delegate represents a place. */
    property var controller: null
    /** The place to display and to navigate to. */
    property var place
    property bool showButtons: true

    /** Indicates that this is the begin (and only the begin) of an element (e.g. departure location of a transit element). */
    property bool isRangeBegin: false
    /** Indicates that this is the end (and only the end) of an element (e.g. arrival location of a transit element). */
    property bool isRangeEnd: false
    /** Indicates that this represents the full range of the element (only valid for non-transit elements). */
    readonly property bool isFullRange: !isRangeBegin && !isRangeEnd

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
        Layout.fillWidth: true
        text: place ? Localizer.formatAddress(place.address) : ""
    }

    Kirigami.ActionToolBar {
        id: buttonLayout
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight
        visible: showButtons

        actions: [
            Kirigami.Action {
                visible: place != undefined && place.geo.isValid
                icon.name: "map-symbolic"
                onTriggered: {
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
                text: i18nc("@action:button", "View Indoor Map")
            },

            Kirigami.Action {
                visible: place != undefined && (place.geo.isValid || !place.address.isEmpty)
                icon.name: "map-globe"
                onTriggered: NavigationController.showOnMap(place)
                text: i18nc("@action:button", "View on Map")
            },

            // navigate to is offered if:
            // - for departures (begins) of any transit element, unless it's a layover from a previous transit element TODO
            // - for begins of any non-transit element
            // - for the end/arrival of non-public transport transit elements (e.g. car rental drop-offs)
            Kirigami.Action {
                visible: NavigationController.canNavigateTo(place) && (!isRangeEnd || (controller.isLocationChange && !controller.isPublicTransport))
                icon.name: "go-next-symbolic"
                onTriggered: {
                    controller.previousLocation ? NavigationController.navigateTo(controller.previousLocation, place) : NavigationController.navigateTo(place);
                }
                text: i18nc("@action:button Start route guidance to location", "Navigate")
            },

            // public transport connections are offered:
            // - at all arrival locations, unless it's a layover to a subsequent transit element TODO
            // -  when leaving non-transit events
            Kirigami.Action {
                visible: place != undefined && place.geo.isValid && !isRangeBegin
                icon.name: "view-calendar-day"
                onTriggered: applicationWindow().pageStack.push(departuresPage)
                text: i18nc("@action:button", "Public Transport Departures")
            }
        ]
    }
}
