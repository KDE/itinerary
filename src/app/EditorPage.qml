/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    property var batchId: controller ? controller.batchId : undefined
    property QtObject controller: null
    property var reservation: ReservationManager.reservation(root.batchId);

    /** Input validation for derived pages. */
    property bool isValidInput: true

    /** Returns the country we are assumed to be in at the given time. */
    function countryAtTime(dt) {
        const place = TimelineModel.locationAtTime(dt);
        if (place && place.address.addressCountry) {
            return place.address.addressCountry;
        }
        return Settings.homeCountryIsoCode;
    }

    leftPadding: 0
    rightPadding: 0

    actions {
        main: Kirigami.Action {
            text: i18n("Save")
            iconName: "document-save"
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
    }
}
