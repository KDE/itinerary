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

    function apply(reservation) {
        var event = reservation.reservationFor;
        var loc = address.save(reservation.reservationFor.location ? reservation.reservationFor.location : Factory.makePlace());
        loc.name = venueName.text;
        event.location = loc;
        event.url = urlEdit.text;

        if (entranceTimeEdit.isModified)
            event = Util.setDateTimePreserveTimezone(event, "doorTime", entranceTimeEdit.value);
        if (startDateEdit.isModified)
            event = Util.setDateTimePreserveTimezone(event, "startDate", startDateEdit.value);
        if (endDateEdit.isModified)
            event = Util.setDateTimePreserveTimezone(event, "endDate", endDateEdit.value);

        var newRes = reservation;
        newRes.reservationFor = event;
        return newRes;
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

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18nc("event as in concert/conference/show, not as in appointment", "Event")
                }
                App.FormDateTimeEditDelegate {
                    id: entranceTimeEdit
                    text: i18nc("time of entrance", "Entrance")
                    obj: reservation.reservationFor
                    propertyName: "doorTime"
                    initialValue: {
                        let d = new Date(startDateEdit.value);
                        d.setHours(d.getHours() - 2);
                        return d;
                    }
                }
                App.FormDateTimeEditDelegate {
                    id: startDateEdit
                    text: i18n("Start Time")
                    obj: reservation.reservationFor
                    propertyName: "startDate"
                }
                App.FormDateTimeEditDelegate {
                    id: endDateEdit
                    text: i18n("End Time")
                    obj: reservation.reservationFor
                    propertyName: "endDate"
                }
                MobileForm.FormTextFieldDelegate {
                    id: urlEdit
                    label: i18n("Website")
                    text: reservationFor.url
                    inputMethodHints: Qt.ImhEmailCharactersOnly
                }
            }
        }

        // TODO seat
    }
}
