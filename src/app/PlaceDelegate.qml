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

    /** The place to display and to navigate to. */
    property var place
    /** Disable navigation actions (makes sense for destinations). */
    property bool offerNavigation: true
    /** The start location for navigation.
     *  If unspecified, the current location is used.
     */
    property var currentLocation

    implicitHeight: (!place.address.isEmpty || place.geo.isValid) ? Math.max(buttonLayout.implicitHeight, label.implicitHeight) : 0
    implicitWidth: label.width + buttonLayout.width

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
        QQC2.ToolButton {
            visible: offerNavigation && _appController.canNavigateTo(place)
            icon.name: "go-next-symbolic"
            onClicked: {
                console.log(currentLocation);
                (currentLocation == undefined || currentLocation == null) ? _appController.navigateTo(place) : _appController.navigateTo(currentLocation, place);
            }
        }
    }
}
