// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    readonly property var touristAttraction: ReservationManager.reservation(root.batchId).touristAttraction
    title: i18n("Tourist Attraction")

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: touristAttraction.name
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                App.FormPlaceDelegate {
                    id: departureDelegate
                    place: touristAttraction
                    controller: root.controller
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // arrival data
                MobileForm.FormCardHeader {
                    title: i18n("Departure")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Depature Time")
                    description: Localizer.formatDateTime(reservation, "departureTime")
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // arrival data
                MobileForm.FormCardHeader {
                    title: i18n("Arrival")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Arrival Time")
                    description: Localizer.formatDateTime(reservation, "arrivalTime")
                }
            }
        }
    }
}

