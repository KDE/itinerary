/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

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
        importController: ImportController
        onSearchSucceeded: applicationWindow().pageStack.goBack();
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
