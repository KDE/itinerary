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
        }
        FormCard.FormDelegateSeparator {
            visible: membershipNumberLabel.visible
        }
        FormCard.FormButtonDelegate {
            id: passButton
            text: i18n("Show program pass")
            property string passId: PassManager.findMatchingPass(root.programMembership)
            visible: passId
            onClicked: applicationWindow().pageStack.push(programMembershipPage, {
                programMembership: PassManager.pass(passButton.passId),
                passId: passButton.passId,
            })
        }
    }
}
