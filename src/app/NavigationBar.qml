/*
    SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.itinerary

Kirigami.NavigationTabBar {
    id: root

    visible: pageStack.layers.depth < 2
    actions: [
        Kirigami.PagePoolAction {
            icon.name: "format-list-unordered"
            text: i18nc("@title:tab This isn't the app name, but the English noun", "Itinerary")
            pagePool: pagepool
            page: Settings.developmentMode ?  Qt.resolvedUrl("TripGroupsPage.qml") :  Qt.resolvedUrl("TimelinePage.qml")
        },
        Kirigami.Action {
            icon.name: "search"
            text: i18nc("@title:tab", "Plan Trip")
            onTriggered: {
                if (pageStack.currentItem as JourneyRequestPage) {
                    return;
                }
                if (Settings.developmentMode) {
                    if (pageStack.currentItem as TripGroupPage) {
                        pageStack.currentItem.addTrainTrip();
                    } else {
                        pageStack.clear();
                        pageStack.push(Qt.resolvedUrl("JourneyRequestPage.qml"), {
                            publicTransportManager: LiveDataManager.publicTransportManager,
                            initialCountry: Settings.homeCountryIsoCode
                        });
                    }
                } else {
                    pagepool.loadPage(Qt.resolvedUrl("TimelinePage.qml")).addTrainTrip()
                }
            }
        },
        Kirigami.PagePoolAction {
            icon.name: "wallet-open"
            text: i18nc("@title:tab", "Passes & Programs")
            pagePool: pagepool
            page: Qt.resolvedUrl("PassPage.qml")
        },
        Kirigami.Action {
            text: i18nc("@title:tab", "Current Ticket")
            icon.name: "view-barcode-qr"
            visible: TripGroupModel.currentBatchId !== ""
            onTriggered: {
                if (Settings.developmentMode)
                    pagepool.loadPage(Qt.resolvedUrl("TripGroupsPage.qml")).openCurrentReservation()
                else
                    pagepool.loadPage(Qt.resolvedUrl("TimelinePage.qml")).showDetailsPageForReservation(TripGroupModel.currentBatchId)
            }
        }
    ]
}
