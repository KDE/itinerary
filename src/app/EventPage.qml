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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Event")

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        App.TicketTokenDelegate {
            resIds: _reservationManager.reservationsForBatch(root.batchId)
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true


            QQC2.Label {
                Kirigami.FormData.label: i18n("Location:")
                text: reservationFor.location.name
            }

            App.PlaceDelegate {
                place: reservationFor.location
                controller: root.controller
            }

            QQC2.Label {
                Kirigami.FormData.label: i18n("Entrance:")
                text: Localizer.formatDateTime(reservationFor, "doorTime")
                visible: reservationFor.doorTime > 0
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Start Time:")
                text: Localizer.formatDateTime(reservationFor, "startDate")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("End Time:")
                text: Localizer.formatDateTime(reservationFor, "endDate")
                visible: reservationFor.endDate > 0
            }
        }
    }
}
