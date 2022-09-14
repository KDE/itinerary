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

App.DetailsPage {
    id: root
    title: i18nc("event as in concert/conference/show, not as in appointment", "Event")
    editor: Component {
        App.EventEditor {
            batchId: root.batchId
        }
    }

    actions.main: Kirigami.Action {
        icon.name: "view-barcode-qr"
        text: i18n("Barcode Scan Mode")
        onTriggered: scanModeController.toggle()
        visible: ticketToken.hasBarcode
        checkable: true
        checked: scanModeController.enabled
    }

    BarcodeScanModeController {
        id: scanModeController
        page: root
    }

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
            wrapMode: Text.WordWrap
        }

        App.TicketTokenDelegate {
            id: ticketToken
            resIds: ReservationManager.reservationsForBatch(root.batchId)
            onScanModeToggled: scanModeController.toggle()
            onCurrentReservationIdChanged: {
                if (!currentReservationId)
                    return;
                root.currentReservationId = currentReservationId;
            }
        }
        Kirigami.FormLayout {
            Layout.fillWidth: true


            QQC2.Label {
                Kirigami.FormData.label: i18n("Location:")
                text: reservationFor.location != undefined ? reservationFor.location.name : ""
                visible: reservationFor.location != undefined
            }

            App.PlaceDelegate {
                place: reservationFor.location
                controller: root.controller
                visible: reservationFor.location != undefined
            }

            QQC2.Label {
                Kirigami.FormData.label: i18nc("time of entrance", "Entrance:")
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
