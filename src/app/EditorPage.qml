/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kitinerary
import org.kde.itinerary
import "." as App

Kirigami.ScrollablePage {
    id: root

    property var batchId: controller ? controller.batchId : undefined
    property QtObject controller: null
    property var reservation: ReservationManager.reservation(root.batchId);

    property T.Action saveAction: QQC2.Action {
        text: i18nc("@action:button", "Save")
        icon.name: "document-save"
        enabled: root.isValidInput
        onTriggered: {
            const newRes = root.apply(root.reservation);
            if (root.batchId) { // update to an existing element
                ReservationManager.updateReservation(root.batchId, newRes);
            } else { // newly added element
                ReservationManager.importReservation(newRes);
            }
            pageStack.pop();
        }
    }

    /** Input validation for derived pages. */
    property bool isValidInput: true

    /** Returns the city/region/country we are assumed to be in at the given time. */
    function cityAtTime(dt) {
        let city = Factory.makePlace();
        let addr = city.address;
        const loc = TimelineModel.locationAtTime(dt);
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

    footer: QQC2.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                action: saveAction
            }
        }
    }
}
