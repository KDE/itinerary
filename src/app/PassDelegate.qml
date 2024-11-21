// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormButtonDelegate {
    required property var pass
    required property string passId
    required property int type
    required property string validRangeLabel
    readonly property string pkPassId: PkPassManager.passId(pass)
    readonly property var pkPass: PkPassManager.pass(pkPassId)

    icon.name: pkPassId !== "" ? "image://org.kde.pkpass/" + pkPassId + "/icon" : ReservationHelper.defaultIconName(pass);

    text: switch (type) {
    case PassManager.ProgramMembership:
        return pass.programName;
    case PassManager.PkPass:
        return pkPass.description;
    case PassManager.Ticket:
        return pass.name;
    }

    description: switch (type) {
    case PassManager.ProgramMembership:
        if (!pass.member.name)
            return pass.membershipNumber;
        if (!pass.membershipNumber)
            return pass.member.name;
        return i18nc("name - number", "%1 - %2", pass.member.name, pass.membershipNumber)
    case PassManager.PkPass:
        return pkPass.organizationName;
    case PassManager.Ticket:
        if (pass.underName.name === "")
            return validRangeLabel;
        if (validRangeLabel === "")
            return pass.underName.name;
        return i18nc("name - valid time range", "%1 - %2", pass.underName.name, validRangeLabel);
    }

    onClicked: switch (type) {
    case PassManager.ProgramMembership:
        const programMembershipPage = Qt.createComponent("org.kde.itinerary", "ProgramMembershipPage");
        QQC2.ApplicationWindow.window.pageStack.push(programMembershipPage, {
            programMembership: pass,
            passId: passId,
        });
        return;
    case PassManager.PkPass:
        const pkpassComponent = Qt.createComponent("org.kde.itinerary", "GenericPkPassPage");
        QQC2.ApplicationWindow.window.pageStack.push(pkpassComponent, {
            passId: pkPassId,
            pass: pkPass,
            genericPassId: passId,
        });
        return;
    case PassManager.Ticket:
        const ticketComponent = Qt.createComponent("org.kde.itinerary", "TicketPage");
        QQC2.ApplicationWindow.window.pageStack.push(ticketComponent, {
            ticket: pass,
            passId: passId,
        });
        return;
    }
}
