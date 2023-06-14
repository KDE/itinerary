// SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.itinerary 1.0

MobileForm.FormCard {
    id: root

    property var programMembership

    visible: programNameLabel.visible || membershipNumberLabel.visible
    Layout.topMargin: Kirigami.Units.largeSpacing
    Layout.fillWidth: true

    contentItem: ColumnLayout {
        MobileForm.FormCardHeader {
            title: i18nc("bonus, discount or frequent traveler program", "Program")
        }
        MobileForm.FormTextDelegate {
            id: programNameLabel
            text: i18n("Name")
            description: root.programMembership.programName
            visible: description
        }
        MobileForm.FormDelegateSeparator {
            visible: programNameLabel.visible
        }
        MobileForm.FormTextDelegate {
            id: membershipNumberLabel
            text: i18n("Number")
            description: root.programMembership.membershipNumber
            visible: description
        }
        MobileForm.FormDelegateSeparator {
            visible: membershipNumberLabel.visible
        }
        MobileForm.FormButtonDelegate {
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
