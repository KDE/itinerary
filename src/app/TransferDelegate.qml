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

Kirigami.AbstractCard {
    id: root
    property var transfer
    property bool journeyDetailsExpanded: false

   header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.rightMargin: -root.rightPadding

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "qrc:///images/transfer.svg"
                width: Kirigami.Units.iconSizes.small
                height: width
                color: Kirigami.Theme.textColor
                isMask: true
            }
            QQC2.Label {
                text: i18n("%1 to %2", transfer.fromName, transfer.toName)
                color: Kirigami.Theme.textColor
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: Localizer.formatTime(transfer.journey, "scheduledDepartureTime")
                visible: transfer.state == Transfer.Selected
                color: Kirigami.Theme.textColor
            }
            QQC2.Label {
                text: (transfer.journey.departureDelay >= 0 ? "+" : "") + transfer.journey.departureDelay
                color: (transfer.journey.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: transfer.state == Transfer.Selected && transfer.journey.hasExpectedDepartureTime
            }
        }
    }

    contentItem: ColumnLayout {
        ListView {
            delegate: App.JourneySectionDelegate{}
            model: (transfer.state == Transfer.Selected && journeyDetailsExpanded) ? transfer.journey.sections : 0
            implicitHeight: contentHeight
            Layout.fillWidth: true
            boundsBehavior: Flickable.StopAtBounds
        }
        App.JourneySummaryDelegate {
            journey: transfer.journey
            visible: transfer.state == Transfer.Selected && !journeyDetailsExpanded
            Layout.fillWidth: true
        }
        QQC2.Button {
            text: i18n("Select...")
            visible: transfer.state == Transfer.Selected && journeyDetailsExpanded
            onClicked: applicationWindow().pageStack.push(detailsComponent);
        }
        RowLayout {
            visible: transfer.state == Transfer.Pending
            QQC2.Label {
                text: i18n("Select...")
                Layout.fillWidth: true
            }
            QQC2.ToolButton {
                icon.name: "edit-delete"
                onClicked: TransferManager.discardTransfer(transfer)
            }
        }
    }

    Component {
        id: detailsComponent
        App.TransferPage {
            transfer: root.transfer
        }
    }

    onClicked: {
        if (transfer.state == Transfer.Selected) {
            journeyDetailsExpanded = !journeyDetailsExpanded;
        } else {
            applicationWindow().pageStack.push(detailsComponent);
        }
    }
}
