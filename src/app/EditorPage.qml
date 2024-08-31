/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kitinerary
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    property var batchId: controller ? controller.batchId : undefined
    property QtObject controller: null
    property var reservation: ReservationManager.reservation(root.batchId);

    /** This is creating a new element, rather than editing an existing one. */
    readonly property bool isNew: batchId ? false : true

    property T.Action saveAction: QQC2.Action {
        text: i18nc("@action:button", "Save")
        icon.name: "document-save"
        enabled: root.isValidInput && (!root.isNew || !root.tripGroupSelector || root.tripGroupSelector.isValidInput || !Settings.developmentMode)
        onTriggered: {
            const newRes = root.apply(root.reservation);
            if (!root.isNew) { // update to an existing element
                ReservationManager.updateReservation(root.batchId, newRes);
            } else { // newly added element
                if (!root.tripGroupSelector || !Settings.developmentMode) {
                    ReservationManager.addReservationWithPostProcessing(newRes);
                } else {
                    let tgId = "";
                    if (root.tripGroupSelector.mode === TripGroupSelectorCard.Mode.Create) {
                        tgId = TripGroupManager.createEmptyGroup(root.tripGroupSelector.name);
                    } else {
                        tgId = root.tripGroupSelector.tripGroupId;
                    }
                    ApplicationController.addNewReservation(newRes, tgId);
                }
            }
            pageStack.pop();
        }
    }

    /** Input validation for derived pages. */
    property bool isValidInput: true

    /** Trip group selector card, for new reservations, if present. */
    property TripGroupSelectorCard tripGroupSelector: null

    /** Returns the city/region/country we are assumed to be in at the given time. */
    function cityAtTime(dt) {
        let city = Factory.makePlace();
        let addr = city.address;
        const loc = TripGroupModel.locationAtTime(dt);
        if (loc && !loc.address.isEmpty) {
            addr.addressLocality = loc.address.addressLocality;
            addr.addressRegion = loc.address.addressRegion;
            addr.addressCountry = loc.address.addressCountry;
        } else {
            addr.addressCountry = Settings.homeCountryIsoCode;
        }
        city.address = addr;
        return city;
    }

    leftPadding: 0
    rightPadding: 0

    data: FloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: root.saveAction
    }
}
