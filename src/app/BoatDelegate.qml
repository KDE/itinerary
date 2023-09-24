/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    headerIconSource: "qrc:///images/ferry.svg"
    headerItem: RowLayout {
        QQC2.Label {
            text: i18n("%1 to %2", reservationFor.departureBoatTerminal.name, reservationFor.arrivalBoatTerminal.name);
            color: root.headerTextColor
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservationFor, "departureTime")
            color: root.headerTextColor
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18n("From: %1", reservationFor.departureBoatTerminal.name)
            width: topLayout.width
        }
        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.departureBoatTerminal.address,
                                                     reservationFor.arrivalBoatTerminal.address,
                                                     Settings.homeCountryIsoCode)
        }
        Kirigami.Separator {
            width: topLayout.width
        }
        QQC2.Label {
            text: i18n("To: %1", reservationFor.arrivalBoatTerminal.name)
            width: topLayout.width
        }
        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.arrivalBoatTerminal.address,
                                                     reservationFor.departureBoatTerminal.address,
                                                     Settings.homeCountryIsoCode)
        }
        QQC2.Label {
            text: i18n("Arrival time: %1", Localizer.formatDateTime(reservationFor, "arrivalTime"))
            wrapMode: Text.WordWrap
        }
    }

    onClicked: showDetailsPage(boatDetailsPage, root.batchId)
}

