/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    property var transfer;

    headerIconSource: "qrc:///images/transfer.svg"
    headerItem: RowLayout {
        QQC2.Label {
            text: "TODO TRANSFER"
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(transfer.journey, "scheduledDepartureTime")
            visible: transfer.state == Transfer.Valid
            color: Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: (transfer.journey.departureDelay >= 0 ? "+" : "") + transfer.journey.departureDelay
            color: (transfer.journey.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: transfer.state == Transfer.Valid && transfer.journey.hasExpectedDepartureTime
        }
    }

    contentItem: ColumnLayout {
        // TODO
    }

    Component {
        id: detailsComponent
        App.TransferPage {
            transfer: root.transfer
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}
