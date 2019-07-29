/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    property var batchId
    property var resIds: _reservationManager.reservationsForBatch(root.batchId)
    readonly property var reservation: _reservationManager.reservation(root.batchId);

    property alias dateTimeEditSheet: _dateTimeEditSheet

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
