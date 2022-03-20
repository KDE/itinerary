/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    property var batchId: controller ? controller.batchId : undefined
    property QtObject controller: null
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)
    readonly property var reservation: ReservationManager.reservation(root.batchId);

    property alias dateTimeEditSheet: _dateTimeEditSheet

    /** Returns the country we are assumed to be in at the given time. */
    function countryAtTime(dt) {
        const place = TimelineModel.locationAtTime(dt);
        if (place && place.address.addressCountry) {
            return place.address.addressCountry;
        }
        return Settings.homeCountryIsoCode;
    }

    actions {
        main: Kirigami.Action {
            iconName: "document-save"
            onTriggered: {
                root.save(batchId, reservation);
                pageStack.pop();
            }
        }
    }

    Kirigami.OverlaySheet {
        id: _dateTimeEditSheet
        property date value

        parent: root
        header: Kirigami.Heading { text: i18nc("@title:window", "Edit date/time") }

        ColumnLayout {
            App.DateInput {
                id: dateInput
                value: _dateTimeEditSheet.value
            }
            App.TimeInput {
                id: timeInput
                value: _dateTimeEditSheet.value
            }

            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Ok")
                onClicked: {
                    var dt = new Date();
                    dt.setFullYear(dateInput.value.getFullYear());
                    dt.setMonth(dateInput.value.getMonth());
                    dt.setDate(dateInput.value.getDate());
                    dt.setHours(timeInput.value.getHours());
                    dt.setMinutes(timeInput.value.getMinutes(), 0, 0);
                    _dateTimeEditSheet.value = dt;
                    _dateTimeEditSheet.sheetOpen = false;
                }
            }
        }
    }
}
