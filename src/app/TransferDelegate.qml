/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
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
                elide: Text.ElideRight
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

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        ListView {
            delegate: App.JourneySectionDelegate{}
            model: (transfer.state == Transfer.Selected && journeyDetailsExpanded) ? transfer.journey.sections : 0
            implicitHeight: contentHeight
            width: topLayout.width
            boundsBehavior: Flickable.StopAtBounds
        }
        App.JourneySummaryDelegate {
            journey: transfer.journey
            visible: transfer.state == Transfer.Selected && !journeyDetailsExpanded
            width: parent.width
        }
        QQC2.Button {
            text: i18n("Select...")
            visible: transfer.state == Transfer.Selected && journeyDetailsExpanded
            onClicked: applicationWindow().pageStack.push(detailsComponent);
        }
        RowLayout {
            width: topLayout.width
            visible: transfer.state == Transfer.Pending || transfer.state == Transfer.Searching
            QQC2.BusyIndicator {
                running: visible
                visible: transfer.state == Transfer.Searching
            }
            QQC2.Label {
                text: i18n("Select transfer...")
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
