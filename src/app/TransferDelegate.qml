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

    required property transfer transfer
    property bool journeyDetailsExpanded: false
    property QtObject controller: TransferDelegateController {
        transfer: root.transfer
    }
    FormCard.AbstractFormDelegate {
        background: Item {
            id: headerBackground
            Rectangle {
                id: progressBar

                visible: root.controller.isCurrent
                height: Kirigami.Units.smallSpacing
                width: root.controller.progress * headerBackground.width
                color: Kirigami.Theme.visitedLinkColor
                radius: Kirigami.Units.cornerRadius

                anchors {
                    bottom: headerBackground.bottom
                    left: headerBackground.left
                }
            }
        }
        onClicked: {
            if (root.transfer.state === Transfer.Selected) {
                root.journeyDetailsExpanded = !root.journeyDetailsExpanded;
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
                text: i18n("%1 to %2", root.transfer.fromName, root.transfer.toName)
                color: Kirigami.Theme.textColor
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                Accessible.ignored: true
            }
            QQC2.Label {
                text: Localizer.formatTime(root.transfer.journey, "scheduledDepartureTime")
                visible: root.transfer.state === Transfer.Selected && !root.journeyDetailsExpanded
                color: Kirigami.Theme.textColor
                Accessible.ignored: !visible
            }
            QQC2.Label {
                text: (root.transfer.journey.departureDelay >= 0 ? "+" : "") + root.transfer.journey.departureDelay
                color: (root.transfer.journey.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: root.transfer.state === Transfer.Selected && root.transfer.journey.hasExpectedDepartureTime && !root.journeyDetailsExpanded
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
        model: (root.transfer.state == Transfer.Selected && root.journeyDetailsExpanded) ? root.transfer.journey.sections : 0
    }

    JourneySummaryDelegate {
        journey: root.transfer.journey
        visible: root.transfer.state == Transfer.Selected && !root.journeyDetailsExpanded
        Layout.fillWidth: true
        onClicked: root.journeyDetailsExpanded = true
    }

    FormCard.FormButtonDelegate {
        icon.name: "checkmark"
        text: i18n("Select transfer")
        onClicked: applicationWindow().pageStack.push(detailsComponent)
        visible: root.transfer.state === Transfer.Pending
            || root.transfer.state === Transfer.Searching
            || root.transfer.state === Transfer.Selected && root.journeyDetailsExpanded
        Accessible.ignored: !visible
    }

    FormCard.FormButtonDelegate {
        icon.name: "edit-delete"
        text: i18n("Delete transfer")
        onClicked: TransferManager.discardTransfer(root.transfer)
        visible: root.transfer.state == Transfer.Pending || root.transfer.state == Transfer.Searching
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
