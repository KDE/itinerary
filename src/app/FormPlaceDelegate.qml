// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary
import "." as App

/** Display a location and corresponding navigation actions. */
FormCard.AbstractFormDelegate {
    id: root

    property var controller: null
    /** The place to display and to navigate to. */
    property var place
    property bool showLocationName: false

    /** Indicates that this is the begin (and only the begin) of an element (e.g. departure location of a transit element). */
    property bool isRangeBegin: false
    /** Indicates that this is the end (and only the end) of an element (e.g. arrival location of a transit element). */
    property bool isRangeEnd: false
    /** Indicates that this represents the full range of the element (only valid for non-transit elements). */
    readonly property bool isFullRange: !isRangeBegin && !isRangeEnd

    background: null
    visible: place && !place.address.isEmpty
    text: i18n("Location")

    Component {
        id: departuresPage
        App.DepartureQueryPage {
            stop: place
            dateTime: controller.effectiveEndTime
        }
    }

    MatrixRoomSelectionSheet {
        parent: applicationWindow().overlay
        id: matrixRoomSheet
            onRoomSelected: {
            console.log(room);
            shareConfirmDialog.room = room;
            shareConfirmDialog.open();
         }
    }

    Kirigami.PromptDialog {
        id: shareConfirmDialog

        property var room

        title: i18n("Share Place")
        subtitle: room ? i18n("Do you really want to share %1 to the Matrix channel %2?", root.place.name, room.displayName) : ""

        standardButtons: QQC2.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Share")
                icon.name: "emblem-shared-symbolic"
                onTriggered: {
                    console.log(shareConfirmDialog.room.id);
                    MatrixController.manager.postLocation(shareConfirmDialog.room.id, root.place.geo.latitude, root.place.geo.longitude, root.place.name);
                    shareConfirmDialog.close();
                }
            }
        ]
        closePolicy: QQC2.Popup.CloseOnEscape
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        QQC2.Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: root.text
            Accessible.ignored: true
        }
        QQC2.Label {
            visible: showLocationName && root.place != undefined && root.place.name !== undefined
            Layout.fillWidth: true
            text: root.place != undefined ? place.name : ""
            color: Kirigami.Theme.disabledTextColor
            wrapMode: Text.WordWrap
            Accessible.ignored: !visible
        }
        QQC2.Label {
            visible: root.place != undefined && !root.place.address.isEmpty
            Layout.fillWidth: true
            text: root.place ? Localizer.formatAddress(root.place.address) : ""
            color: Kirigami.Theme.disabledTextColor
            wrapMode: Text.WordWrap
            Accessible.ignored: !visible
        }

        Kirigami.ActionToolBar {
            id: buttonLayout
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            visible: root.place != undefined && (root.place.geo.isValid || !root.place.address.isEmpty)

            actions: [
                Kirigami.Action {
                    visible: root.place != undefined && root.place.geo.isValid
                    icon.name: "map-symbolic"
                    onTriggered: {
                        var args = {};
                        if (root.controller.isLocationChange) {
                            if (root.isRangeBegin) {
                                args = root.controller.departureMapArguments();
                            } else if (root.isRangeEnd) {
                                args = root.controller.arrivalMapArguments();
                            }
                        } else {
                            args = root.controller.mapArguments();
                        }
                        console.log(JSON.stringify(args));
                        applicationWindow().pageStack.push(indoorMapPage, args);
                    }
                    text: i18nc("@action:button", "View Indoor Map")
                },

                Kirigami.Action {
                    visible: root.place != undefined && (root.place.geo.isValid || !root.place.address.isEmpty)
                    icon.name: "map-globe"
                    onTriggered: NavigationController.showOnMap(place)
                    text: i18nc("@action:button", "View on Map")
                },

                // navigate to is offered if:
                // - for departures (begins) of any transit element, unless it's a layover from a previous transit element TODO
                // - for begins of any non-transit element
                // - for the end/arrival of non-public transport transit elements (e.g. car rental drop-offs)
                Kirigami.Action {
                    visible: NavigationController.canNavigateTo(root.place) && (!root.isRangeEnd || (root.controller.isLocationChange && !root.controller.isPublicTransport))
                    icon.name: "go-next-symbolic"
                    onTriggered: {
                        root.controller.previousLocation ? NavigationController.navigateTo(root.controller.previousLocation, root.place) : NavigationController.navigateTo(root.place);
                    }
                    text: i18nc("@action:button Start route guidance to location", "Navigate")
                },

                // public transport connections are offered:
                // - at all arrival locations, unless it's a layover to a subsequent transit element TODO
                // -  when leaving non-transit events
                Kirigami.Action {
                    visible: root.place != undefined && root.place.geo.isValid && !root.isRangeBegin
                    icon.name: "view-calendar-day"
                    onTriggered: applicationWindow().pageStack.push(departuresPage)
                    text: i18nc("@action:button", "Public Transport Departures")
                },

                // share in Matrix channel
                Kirigami.Action {
                    visible: MatrixController.isAvailable && root.place != undefined && root.place.geo.isValid && root.place.name
                    enabled: MatrixController.manager.connected
                    icon.name: "emblem-shared-symbolic"
                    onTriggered: matrixRoomSheet.open()
                    text: i18nc("@action:button", "Share via Matrix")
                }
            ]
        }
    }
}
