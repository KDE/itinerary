/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.itinerary 1.0
import "." as App

/** Display of a location and navitaion actions. */
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

    implicitHeight: (!place.address.isEmpty || place.geo.isValid) ? Math.max(buttonLayout.implicitHeight, label.implicitHeight) : 0
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
        visible: !place.address.isEmpty
        text: Localizer.formatAddress(place.address)
        color: Kirigami.Theme.textColor
    }

    RowLayout {
        id: buttonLayout
        anchors.right: root.right
        y: Math.max(0, label.implicitHeight - buttonLayout.implicitHeight)

        QQC2.ToolButton {
            visible: place.geo.isValid || !place.address.isEmpty
            icon.name: "map-symbolic"
            onClicked: _appController.showOnMap(place)
        }

        // navigate to is offered if:
        // - for departures (begins) of any transit element, unless it's a layover from a previous transit element TODO
        // - for begins of any non-transit element
        // - for the end/arrival of non-public transport transit elements (e.g. car rental drop-offs)
        QQC2.ToolButton {
            visible: _appController.canNavigateTo(place) && (!isRangeEnd || (controller.isLocationChange && !controller.isPublicTransport))
            icon.name: "go-next-symbolic"
            onClicked: {
                controller.previousLocation ? _appController.navigateTo(controller.previousLocation, place) : _appController.navigateTo(place);
            }
        }

        // public transport connections are offered:
        // - at all arrival locations, unless it's a layover to a subsequent transit element TODO
        // -  when leaving non-transit events
        QQC2.ToolButton {
            visible: place.geo.isValid && !isRangeBegin
            icon.name: "view-calendar"
            onClicked: {
                applicationWindow().pageStack.push(departuresPage);
            }
        }
    }
}
