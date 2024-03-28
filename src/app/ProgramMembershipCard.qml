// SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

ColumnLayout {
    id: root

    required property var programMembership

    spacing: 0

    Layout.fillWidth: true

    FormCard.FormHeader {
        title: i18nc("bonus, discount or frequent traveler program", "Program")
        visible: programNameLabel.visible || membershipNumberLabel.visible
    }

    FormCard.FormCard {
        visible: programNameLabel.visible || membershipNumberLabel.visible

        FormCard.FormTextDelegate {
            id: programNameLabel
            text: i18n("Name")
            description: root.programMembership.programName
            visible: description
        }
        FormCard.FormDelegateSeparator {
            visible: programNameLabel.visible
        }
        FormCard.FormTextDelegate {
            id: membershipNumberLabel
            text: i18n("Number")
            description: root.programMembership.membershipNumber
            visible: description

            trailing: QQC2.ToolButton {
                display: QQC2.AbstractButton.IconOnly
                text: i18nc("@info:tooltip", "Copy to Clipboard")
                icon.name: "edit-copy"
                onClicked: {
                    Clipboard.saveText(programMembership.membershipNumber);
                    applicationWindow().showPassiveNotification(i18n("Program membership number copied to clipboard"));
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }
        FormCard.FormDelegateSeparator {
            visible: membershipNumberLabel.visible
        }
        FormCard.FormButtonDelegate {
            id: passButton
            text: i18n("Show program pass")
            property string passId: PassManager.findMatchingPass(root.programMembership)
            visible: passId
            onClicked: {
                const programMembershipPage = Qt.createComponent("org.kde.itinerary", "ProgramMembershipPage");
                applicationWindow().pageStack.push(programMembershipPage, {
                    programMembership: PassManager.pass(passButton.passId),
                    passId: passButton.passId,
                });
            }
        }
    }
}
