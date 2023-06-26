/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

Kirigami.AbstractCard {
    id: root

    required property var transfer
    property bool journeyDetailsExpanded: false
    property QtObject controller: TransferDelegateController {
        transfer: root.transfer
    }

    header: TimelineDelegateHeaderBackground {
        id: headerBackground
        card: root
        Kirigami.Theme.colorSet: controller.isCurrent ? Kirigami.Theme.Selection : Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        defaultColor: transfer.isReachable ? Kirigami.Theme.backgroundColor : Kirigami.Theme.negativeBackgroundColor
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "qrc:///images/transfer.svg"
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Layout.preferredWidth
                color: Kirigami.Theme.textColor
                isMask: true
            }
            QQC2.Label {
                id: headerLabel
                text: i18n("%1 to %2", transfer.fromName, transfer.toName)
                color: Kirigami.Theme.textColor
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                Accessible.ignored: true
            }
            QQC2.Label {
                text: Localizer.formatTime(transfer.journey, "scheduledDepartureTime")
                visible: transfer.state == Transfer.Selected
                color: Kirigami.Theme.textColor
                Accessible.ignored: !visible
            }
            QQC2.Label {
                text: (transfer.journey.departureDelay >= 0 ? "+" : "") + transfer.journey.departureDelay
                color: (transfer.journey.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: transfer.state == Transfer.Selected && transfer.journey.hasExpectedDepartureTime
                Accessible.ignored: !visible
            }

            QQC2.BusyIndicator {
                running: visible
                visible: transfer.state == Transfer.Searching
                Accessible.ignored: !visible
            }
        }

        Rectangle {
            id: progressBar
            visible: controller.isCurrent
            anchors.bottom: headerBackground.bottom
            anchors.left: headerBackground.left
            height: Kirigami.Units.smallSpacing
            width: controller.progress * headerBackground.width
            color: Kirigami.Theme.visitedLinkColor
        }
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: Item {
        implicitHeight: topLayout.implicitHeight

        ColumnLayout {
            id: topLayout

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }

            spacing: 0

            Repeater {
                delegate: App.JourneySectionDelegate{
                    Layout.fillWidth: true
                }
                model: (transfer.state == Transfer.Selected && journeyDetailsExpanded) ? transfer.journey.sections : 0
            }

            App.JourneySummaryDelegate {
                journey: transfer.journey
                visible: transfer.state == Transfer.Selected && !journeyDetailsExpanded
                Layout.fillWidth: true
                onClicked: journeyDetailsExpanded = true
            }

            MobileForm.FormButtonDelegate {
                icon.name: "checkbox"
                text: i18n("Select transfer")
                onClicked: root.clicked()
                visible: (transfer.state === Transfer.Pending || transfer.state === Transfer.Searching)
                Accessible.ignored: !visible
            }

            MobileForm.FormButtonDelegate {
                icon.name: "edit-delete"
                text: i18n("Delete transfer")
                onClicked: TransferManager.discardTransfer(transfer)
                visible: transfer.state == Transfer.Pending || transfer.state == Transfer.Searching
                Accessible.ignored: !visible
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

    Accessible.name: headerLabel.text
}
