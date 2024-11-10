// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as T
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    required property bool isEmptyTripGroup
    required property ListView listView

    title: i18nc("@title:window", "Add to Trip")

    function addTrainTrip(): void {
        const HOUR = 60 * 60 * 1000;
        let roundInterval = HOUR;
        let dt = new Date();

        if (!root.isEmptyTripGroup) {
            // find date/time at the current screen center
            const idx = currentIndex();

            if (listView.model.data(idx, TimelineModel.IsTimeboxedRole) && !listView.model.data(idx, TimelineModel.IsCanceledRole)) {
                dt = listView.model.data(idx, TimelineModel.EndDateTimeRole);
                roundInterval = 5 * 60 * 1000;
            } else {
                dt = listView.model.data(idx, TimelineModel.StartDateTimeRole);
            }
        }

        // clamp to future times and round to the next plausible hour
        const now = new Date();
        if (!dt || dt.getTime() < now.getTime()) {
            dt = now;
        }
        if (dt.getTime() % HOUR == 0 && dt.getHours() == 0) {
            dt.setTime(dt.getTime() + HOUR * 8);
        } else {
            dt.setTime(dt.getTime() + roundInterval - (dt.getTime() % roundInterval));
        }

        // determine where we are at that time
        const place = TripGroupModel.locationAtTime(dt);
        var country = Settings.homeCountryIsoCode;
        var departureLocation;
        if (place) {
            country = place.address.addressCountry;
            departureLocation = PublicTransport.locationFromPlace(place, undefined);
            departureLocation.name = place.name;
        }

        root.Window.window.pageStack.push(Qt.resolvedUrl("JourneyRequestPage.qml"), {
            publicTransportManager: LiveDataManager.publicTransportManager,
            initialCountry: country,
            initialDateTime: dt,
            departureStop: departureLocation
        });
    }

    readonly property list<Kirigami.Action> addActions: [
        Kirigami.Action {
            text: i18n("Add train trip…")
            icon.name: "list-add-symbolic"
            onTriggered: {
                addTrainTrip()
            }
        },
        Kirigami.Action {
            text: i18n("Add flight…")
            icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Air)
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : dateTimeAtIndex(currentIndex());
                let res =  Factory.makeFlightReservation();
                let trip = res.reservationFor;
                trip.departureTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                res.reservationFor = trip;
                root.Window.window.pageStack.push(flightEditorPage, {reservation: res});
            }
        },
        Kirigami.Action {
            text: i18n("Add ferry trip…")
            icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Ferry)
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : dateTimeAtIndex(currentIndex());
                let res =  Factory.makeBoatReservation();
                let trip = res.reservationFor;
                trip.departureTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                res.reservationFor = trip;
                root.Window.window.pageStack.push(boatEditorPage, {reservation: res});
            }
        },
        Kirigami.Action {
            text: i18n("Add accommodation…")
            icon.name: "go-home-symbolic"
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : dateTimeAtIndex(currentIndex());
                let res =  Factory.makeLodgingReservation();
                res.checkinTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 15, 0);
                res.checkoutTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate() + 1, 11, 0);
                root.Window.window.pageStack.push(hotelEditorPage, {reservation: res});
            }
        },
        Kirigami.Action {
            text: i18n("Add event…")
            icon.name: "meeting-attending"
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : dateTimeAtIndex(currentIndex());
                let res = Factory.makeEventReservation();
                let ev = res.reservationFor;
                ev.startDate = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                res.reservationFor = ev;
                root.Window.window.pageStack.push(eventEditorPage, {reservation: res});
            }
        },
        Kirigami.Action {
            text: i18n("Add restaurant…")
            icon.name: KPublicTransport.FeatureType.typeIconName(KPublicTransport.Feature.Restaurant)
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : dateTimeAtIndex(currentIndex());
                let res =  Factory.makeFoodEstablishmentReservation();
                res.startTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 20, 0);
                res.endTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 22, 0);
                root.Window.window.pageStack.push(restaurantEditorPage, {reservation: res});
            }
        }
    ]

    FormCard.FormHeader {
        title: i18n("Import")
    }

    FormCard.FormCard {
        Repeater {
            model: importActions
            FormCard.FormButtonDelegate {
                required property T.Action modelData
                action: modelData
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Add")
    }

    FormCard.FormCard {
        Repeater {
            model: root.addActions
            FormCard.FormButtonDelegate {
                action: modelData
            }
        }
    }
}
