// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore as QtCore
import QtQuick
import QtQuick.Controls as T
import QtQuick.Window
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport as KPublicTransport
import org.kde.calendarcore as KCalendarCore
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

    FileDialog {
        id: importFileDialog
        fileMode: FileDialog.OpenFile
        title: i18n("Import Reservation")
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        // Android has no file type selector, we get the superset of all filters there since Qt6 (apart from "all"),
        // so don't set any filters on Android in order to be able to open everything we can read
        nameFilters:  Qt.platform.os === "android" ?
            [i18n("All Files (*.*)")] :
            [i18n("All Files (*.*)"), i18n("PkPass files (*.pkpass)"), i18n("PDF files (*.pdf)"), i18n("iCal events (*.ics)"), i18n("KDE Itinerary files (*.itinerary)")]
        onAccepted: ImportController.importFromUrl(selectedFile)
    }

    CalendarSelectionSheet {
        id: calendarSelector
        // parent: root.Overlay.overlay
        onCalendarSelected: (calendar) => {
            ImportController.enableAutoCommit = false;
            ImportController.importFromCalendar(calendar);
        }
    }

    Component {
        id: calendarModel
        // needs to be created on demand, after we have calendar access permissions
        KCalendarCore.CalendarListModel {}
    }

    readonly property list<Kirigami.Action> importActions: [
        Kirigami.Action {
            text: i18n("File")
            icon.name: "document-open"
            shortcut: StandardKey.Open
            onTriggered: {
                importFileDialog.open();
            }
        },
        Kirigami.Action {
            text: i18n("Clipboard")
            icon.name: "edit-paste"
            enabled: Clipboard.hasText || Clipboard.hasUrls || Clipboard.hasBinaryData
            shortcut: StandardKey.Paste
            onTriggered: ImportController.importFromClipboard()
        },
        Kirigami.Action {
            text: i18n("Barcode")
            icon.name: "view-barcode-qr"
            onTriggered: {
                root.Window.window.pageStack.layers.push(scanBarcodeComponent);
            }
        },
        Kirigami.Action {
            icon.name: "view-calendar-day"
            text: i18n("Calendar")
            onTriggered: {
                PermissionManager.requestPermission(Permission.ReadCalendar, function() {
                    if (!calendarSelector.model) {
                        calendarSelector.model = calendarModel.createObject(root);
                    }
                    calendarSelector.open();
                })
            }
            visible: KCalendarCore.CalendarPluginLoader.hasPlugin
        },
        // TODO this should not be hardcoded here, but dynamically filled based on what online ticket
        // sources we support
        Kirigami.Action {
            text: i18n("Deutsche Bahn Online Ticket")
            icon.name: "download"
            onTriggered: {
                root.Window.window.pageStack.push(Qt.createComponent("org.kde.itinerary", "OnlineImportPage"), {
                    source: "db",
                });
            }
        },
        Kirigami.Action {
            text: i18n("SNCF Online Ticket")
            icon.name: "download"
            onTriggered: {
                root.Window.window.pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "OnlineImportPage"), {
                    source: "sncf"
                });
            }
        }
    ]

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
        title: i18nc("@title:group", "Import From")
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
