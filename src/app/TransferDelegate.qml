/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
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
        background: Item {
            id: headerBackground
            Rectangle {
                id: progressBar

                visible: controller.isCurrent
                height: Kirigami.Units.smallSpacing
                width: controller.progress * headerBackground.width
                color: Kirigami.Theme.visitedLinkColor
                radius: Kirigami.Units.cornerRadius

                anchors {
                    bottom: headerBackground.bottom
                    left: headerBackground.left
                }
            }
        }
        onClicked: {
            if (transfer.state === Transfer.Selected) {
                journeyDetailsExpanded = !journeyDetailsExpanded;
            } else {
                applicationWindow().pageStack.push(detailsComponent);
            }
        }
        contentItem: RowLayout {
            id: headerLayout

            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: KPublicTransport.JourneySectionMode.modeIconName(KPublicTransport.JourneySection.Transfer)
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Layout.preferredWidth
                Layout.rightMargin: Kirigami.Units.largeSpacing
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
                visible: transfer.state === Transfer.Selected && !root.journeyDetailsExpanded
                color: Kirigami.Theme.textColor
                Accessible.ignored: !visible
            }
            QQC2.Label {
                text: (transfer.journey.departureDelay >= 0 ? "+" : "") + transfer.journey.departureDelay
                color: (transfer.journey.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: transfer.state === Transfer.Selected && transfer.journey.hasExpectedDepartureTime && !root.journeyDetailsExpanded
                Accessible.ignored: !visible
            }

            QQC2.BusyIndicator {
                running: visible
                visible: transfer.state === Transfer.Searching
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
        onClicked: applicationWindow().pageStack.push(detailsComponent)
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
