// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Addons
import org.kde.itinerary

/** Display a location and corresponding navigation actions. */
FormCard.FormButtonDelegate {
    id: root

    property string placeName: ''

    property TimelineDelegateController controller: null
    /** The place to display and to navigate to. */
    property var place
    property bool showLocationName: false

    /** Indicates that this is the begin (and only the begin) of an element (e.g. departure location of a transit element). */
    property bool isRangeBegin: false
    /** Indicates that this is the end (and only the end) of an element (e.g. arrival location of a transit element). */
    property bool isRangeEnd: false
    /** Indicates that this represents the full range of the element (only valid for non-transit elements). */
    readonly property bool isFullRange: !isRangeBegin && !isRangeEnd

    visible: place && !place.address.isEmpty
    text: i18n("Location")
    description: {
        let result = [];
        if (root.placeName.length > 0) {
            result.push(root.placeName);
        }
        if (showLocationName && root.place !== undefined && root.place.name !== undefined) {
            result.push(root.place.name);
        }
        if (root.place !== undefined) {
            result = result.concat(Localizer.formatAddress(root.place.address).split('\n'));
        }
        return result.join('<br>');
    }

    readonly property bool hasContextMenuActions: root.place != undefined && (root.place.geo.isValid || !root.place.address.isEmpty)
    background.visible: root.hasContextMenuActions
    onClicked: {
        if (!root.hasContextMenuActions)
            return;
        const menu = contextMenu.createObject(root);
        menu.popup();
    }

    Component {
        id: departuresPage

        DepartureQueryPage {
            stop: root.place
            dateTime: root.isRangeBegin ? root.controller.startTime : root.controller.effectiveEndTime
        }
    }

    Component {
        id: matrixRoomDialog

        MatrixRoomSelectionSheet {
            parent: root.QQC2.Overlay.overlay
            onRoomSelected: room => {
                console.log("Sharing with", room.id, room.displayName);
                const dialog = shareConfirmDialog.createObject(root, {
                    room: room,
                });
                dialog.openDialog();
             }
         }
    }

    Component {
        id: shareConfirmDialog

        Addons.MessageDialog {
            id: dialog

            required property var room

            title: i18n("Share Place")
            dialogType: Addons.MessageDialog.Warning
            dontShowAgainName: "sharePlaceConfirmation"

            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("Do you really want to share %1 to the Matrix channel %2?", root.place.name, room.displayName)
                wrapMode: Text.WordWrap
            }

            standardButtons: QQC2.Dialog.Cancel | QQC2.Dialog.Ok

            onAccepted: {
                console.log(dialog.room.id);
                MatrixController.manager.postLocation(dialog.room.id, root.place.geo.latitude, root.place.geo.longitude, root.place.name);
                dialog.close();
            }
            onRejected: dialog.close();

            Component.onCompleted: {
                const shareButton = standardButton(QQC2.Dialog.Ok)
                shareButton.text = i18nc("@action:button", "Share");
                shareButton.icon.name = "emblem-shared-symbolic";
            }

            closePolicy: QQC2.Popup.CloseOnEscape
        }
    }

    Component {
        id: contextMenu
        Addons.ConvergentContextMenu {
            parent: root.QQC2.Overlay.overlay

            headerContentItem: Kirigami.Heading {
                text: root.place.name !== undefined ? root.place.name : Localizer.formatAddress(root.place.address)
            }

            Component.onCompleted: if (!Kirigami.Settings.isMobile) {
                // TODO: make it a binding when Kirigami Addons 1.20 is out
                displayMode = Addons.ConvergentContextMenu.Dialog;
            }

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
            }

            Kirigami.Action {
                visible: root.place != undefined && (root.place.geo.isValid || !root.place.address.isEmpty)
                icon.name: "map-globe"
                onTriggered: NavigationController.showOnMap(place)
                text: i18nc("@action:button", "View on Map")
            }

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
            }

            // public transport departures
            Kirigami.Action {
                visible: root.place != undefined && root.place.geo.isValid
                icon.name: "view-calendar-day"
                onTriggered: applicationWindow().pageStack.push(departuresPage)
                text: i18nc("@action:button", "Public Transport Departures")
            }

            // share in Matrix channel
            Kirigami.Action {
                visible: MatrixController.isAvailable && root.place != undefined && root.place.geo.isValid && root.place.name
                enabled: MatrixController.manager.connected
                icon.name: "emblem-shared-symbolic"
                onTriggered: {
                    const dialog = matrixRoomDialog.createObject(root);
                    dialog.open();
                }

                text: i18nc("@action:button", "Share via Matrix")
            }
        }
    }
}
