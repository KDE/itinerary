/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCard {
    id: root
    width: ListView.view.width

    required property var transfer
    property bool journeyDetailsExpanded: false
    property QtObject controller: TransferDelegateController {
        transfer: root.transfer
    }
    FormCard.AbstractFormDelegate {
        background: Rectangle {
            id: headerBackground
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: controller.isCurrent ? Kirigami.Theme.Selection : Kirigami.Theme.Header
            Kirigami.Theme.inherit: false
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
        onClicked: {
            if (transfer.state == Transfer.Selected) {
                journeyDetailsExpanded = !journeyDetailsExpanded;
            } else {
                applicationWindow().pageStack.push(detailsComponent);
            }
        }
        contentItem: RowLayout {
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
    }

    Repeater {
        id: journeyRepeater
        delegate: JourneySectionDelegate{
            Layout.fillWidth: true
            modelLength: journeyRepeater.count - 1

        }
        model: (transfer.state == Transfer.Selected && journeyDetailsExpanded) ? transfer.journey.sections : 0
    }

    JourneySummaryDelegate {
        journey: transfer.journey
        visible: transfer.state == Transfer.Selected && !journeyDetailsExpanded
        Layout.fillWidth: true
        onClicked: journeyDetailsExpanded = true
    }

    FormCard.FormButtonDelegate {
        icon.name: "checkmark"
        text: i18n("Select transfer")
        onClicked: if (transfer.state === Transfer.Selected) {
            applicationWindow().pageStack.push(detailsComponent)
        } else {
            root.clicked()
        }
        visible: transfer.state === Transfer.Pending
            || transfer.state === Transfer.Searching
            || transfer.state === Transfer.Selected && journeyDetailsExpanded
        Accessible.ignored: !visible
    }

    FormCard.FormButtonDelegate {
        icon.name: "edit-delete"
        text: i18n("Delete transfer")
        onClicked: TransferManager.discardTransfer(transfer)
        visible: transfer.state == Transfer.Pending || transfer.state == Transfer.Searching
        Accessible.ignored: !visible
    }
    Item{
        Component {
            id: detailsComponent
            TransferPage {
                transfer: root.transfer
            }
        }
    }

    Accessible.name: headerLabel.text
}
