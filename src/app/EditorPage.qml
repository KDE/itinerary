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
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)
    readonly property var reservation: ReservationManager.reservation(root.batchId);

    /** Returns the country we are assumed to be in at the given time. */
    function countryAtTime(dt) {
        const place = TimelineModel.locationAtTime(dt);
        if (place && place.address.addressCountry) {
            return place.address.addressCountry;
        }
        return Settings.homeCountryIsoCode;
    }

    actions {
        main: Kirigami.Action {
            text: i18n("Save")
            iconName: "document-save"
            onTriggered: {
                root.save(batchId, reservation);
                pageStack.pop();
            }
        }
    }
}
