/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

KPublicTransport.StopPickerPage {
    id: root

    TripGroupLocationModel {
        id: tripGroupLocationModel
        tripGroupManager: TripGroupManager
        tripGroupId: ApplicationController.contextTripGroupId
        onLocationsChanged: {
            root.locationHistoryModel.clearPresetLocations();
            for (let i = 0; i < tripGroupLocationModel.rowCount(); ++i) {
                const idx = tripGroupLocationModel.index(i, 0);
                root.locationHistoryModel.addPresetLocation(
                    tripGroupLocationModel.data(idx, TripGroupLocationModel.LocationRole),
                    tripGroupLocationModel.data(idx, TripGroupLocationModel.LastUsedRole),
                    tripGroupLocationModel.data(idx, TripGroupLocationModel.UseCountRole)
                );
            }
        }
    }
}
