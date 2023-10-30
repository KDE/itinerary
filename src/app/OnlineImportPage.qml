/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.itinerary 1.0
import "." as App

FormCard.FormCardPage {
    id: root
    title: i18n("Import Online Ticket")

    property string source

    function search() {
        searchButton.description = '';
        importer.search(root.source, vendorInputForm.item.arguments);
    }

    data: OnlineTicketImporter {
        id: importer
        reservationManager: ReservationManager
        onSearchSucceeded: {
            ApplicationController.infoMessage(i18n("Ticket imported"));
            applicationWindow().pageStack.goBack();
        }
        onSearchFailed: {
            searchButton.description = importer.errorMessage === "" ? i18n("No ticket found") : importer.errorMessage;
        }
    }

    FormCard.FormHeader {
        title: i18n("Online Ticket")
    }

    FormCard.FormCard {
        Loader {
            id: vendorInputForm
            Layout.fillWidth: true
            source: root.source !== "" ? Qt.resolvedUrl("onlineimport/" + root.source + ".qml") : null
            enabled: !importer.searching
            Connections {
                target: vendorInputForm.item
                function onSearch() { root.search(); }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            id: searchButton
            text: i18n("Search...")
            icon.name: importer.searching ? "view-refresh" : "search"
            enabled: !importer.searching && vendorInputForm.item.arguments !== undefined
            onClicked: root.search()
        }
    }
}
