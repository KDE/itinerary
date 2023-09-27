/*
    SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.kirigami 2.20 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.NavigationTabBar {
    id: root

    visible: pageStack.layers.depth < 2
    actions: [
        Kirigami.PagePoolAction {
            iconName: "format-list-unordered"
            text: i18n("Itinerary")
            pagePool: pagepool
            page: Qt.resolvedUrl("TimelinePage.qml")
        },
        Kirigami.Action {
            iconName: "search"
            text: i18n("Plan Trip")
            onTriggered: {
                if (pageStack.currentItem.objectName !== "JourneyRequestPage") {
                    pagepool.loadPage("TimelinePage.qml").addTrainTrip()
                }
            }
        },
        Kirigami.PagePoolAction {
            iconName: "wallet-open"
            text: i18n("Passes & Programs")
            pagePool: pagepool
            page: Qt.resolvedUrl("PassPage.qml")
        },
        Kirigami.Action {
            text: i18n("Current Ticket")
            icon.name: "view-barcode-qr"
            visible: TimelineModel.currentBatchId !== ""
            onTriggered: pagepool.loadPage("TimelinePage.qml").showDetailsPageForReservation(TimelineModel.currentBatchId)
        },
        Kirigami.PagePoolAction {
            iconName: "view-statistics"
            text: i18n("Statistics")
            pagePool: pagepool
            page: Qt.resolvedUrl("StatisticsPage.qml")
            initialProperties: {
                "reservationManager": ReservationManager,
                "tripGroupManager": TripGroupManager
            }
        }
    ]
}
