/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18nc("event as in concert/conference/show, not as in appointment", "Edit Event")

    function save(resId, reservation) {
        var event = reservation.reservationFor;
        var loc = address.save(reservation.reservationFor.location ? reservation.reservationFor.location : Factory.makePlace());
        loc.name = venueName.text;
        event.location = loc;
        var newRes = reservation;
        newRes.reservationFor = event;
        ReservationManager.updateReservation(resId, newRes);
    }

    ColumnLayout {
        width: root.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: reservationFor.name
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Venue")
                }
                MobileForm.FormTextFieldDelegate {
                    id: venueName
                    label: i18nc("venue name", "Name")
                    text: reservation.reservationFor.location ? reservation.reservationFor.location.name : ""
                }
                MobileForm.FormDelegateSeparator {}
                App.FormPlaceEditorDelegate {
                    id: address
                    place: reservation.reservationFor.location ? reservation.reservationFor.location : Factory.makePlace()
                    defaultCountry: countryAtTime(reservation.reservationFor.startDate)
                }
            }
        }

        // TODO start/end/entrance times, seat
    }
}
