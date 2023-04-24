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
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservationFor, "departureTime")
            color: root.headerTextColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18n("From: %1", reservationFor.departureBoatTerminal.name)
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.departureBoatTerminal
            controller: root.controller
            isRangeBegin: true
            width: topLayout.width
            showButtons: false
        }
        Kirigami.Separator {
            width: topLayout.width
        }
        QQC2.Label {
            text: i18n("To: %1", reservationFor.arrivalBoatTerminal.name)
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalBoatTerminal
            controller: root.controller
            isRangeEnd: true
            width: topLayout.width
            showButtons: false
        }
        QQC2.Label {
            text: i18n("Arrival time: %1", Localizer.formatDateTime(reservationFor, "arrivalTime"))
            wrapMode: Text.WordWrap
        }
    }

    onClicked: showDetailsPage(boatDetailsPage, root.batchId)
}

