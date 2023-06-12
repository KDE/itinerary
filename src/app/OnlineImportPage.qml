/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Import Online Ticket")

    // TODO instead of a fixed dialog we will likely need per-source UI here
    // e.g. by loading corresponding QML into this page dynamically
    property string source

    OnlineTicketImporter {
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

    ColumnLayout {
        width: root.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Online Ticket")
                }
                MobileForm.FormTextFieldDelegate {
                    id: nameInput
                    label: i18n("Family name")
                    // TODO can we prefill this with the user name
                    text: Settings.read("OnlineImport/Name", "")
                    enabled: !importer.searching
                    onEditingFinished: Settings.write("OnlineImport/Name", nameInput.text)
                }
                MobileForm.FormTextFieldDelegate {
                    id: bookingReferenceInput
                    label: i18n("Booking reference")
                    placeholderText: "ABC123"
                    enabled: !importer.searching
                }
                MobileForm.FormButtonDelegate {
                    id: searchButton
                    text: i18n("Search...")
                    icon.name: importer.searching ? "view-refresh" : "search"
                    enabled: nameInput.text !== "" && bookingReferenceInput.text.length == 6 && !importer.searching
                    onClicked: {
                        description = '';
                        importer.search(root.source, {name: nameInput.text, reference: bookingReferenceInput.text});
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (nameInput.text === "") {
            nameInput.forceActiveFocus();
        } else {
            bookingReferenceInput.forceActiveFocus();
        }
    }
}
