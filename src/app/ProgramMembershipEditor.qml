/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Edit Program Membership")

    required property string passId
    required property var programMembership

    readonly property bool isValidInput: programNameEdit.text !== '' && (!validFromEdit.hasValue || !validUntilEdit.hasValue || validFromEdit.value < validUntilEdit.value)

    actions.main: Kirigami.Action {
        text: i18n("Save")
        icon.name: "document-save"
        enabled: root.isValidInput
        onTriggered: {
            let program = root.passId !== "" ? PassManager.pass(root.passId) : root.programMembership;

            let member = program.member;
            member.name = memberNameEdit.text;
            program.member = member;
            program.programName = programNameEdit.text;
            program.membershipNumber = numberEdit.text;
            if (validFromEdit.isModified)
                program = Util.setDateTimePreserveTimezone(program, "validFrom", validFromEdit.value);
            if (validUntilEdit.isModified)
                program = Util.setDateTimePreserveTimezone(program, "validUntil", validUntilEdit.value);

            if (root.passId !== "")
                PassManager.update(root.passId, program);
            else
                PassManager.import(program);
            applicationWindow().pageStack.pop();
        }
    }

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Program")
                }

                MobileForm.FormTextFieldDelegate {
                    id: programNameEdit
                    label: i18n("Name")
                    text: programMembership.programName
                    status: Kirigami.MessageType.Error
                    statusMessage: text === "" ? i18n("Program name must not be empty.") : ""
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Membership")
                }

                MobileForm.FormTextFieldDelegate {
                    id: memberNameEdit
                    label: i18n("Member")
                    text: programMembership.member.name
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: numberEdit
                    label: i18n("Number")
                    text: programMembership.membershipNumber
                }
                MobileForm.FormDelegateSeparator {}
                // TODO date-only edit delegates
                App.FormDateTimeEditDelegate {
                    id: validFromEdit
                    text: i18n("Valid from")
                    obj: root.programMembership
                    propertyName: "validFrom"
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
                    id: validUntilEdit
                    text: i18n("Valid until")
                    obj: root.programMembership
                    propertyName: "validUntil"
                    initialValue: {
                        if (!validFromEdit.hasValue)
                            return new Date();
                        let d = new Date(validFromEdit.value);
                        d.setFullYear(d.getFullYear() + 1);
                        return d;
                    }
                    status: Kirigami.MessageType.Error
                    statusMessage: {
                        if (validUntilEdit.hasValue && validFromEdit.hasValue && validUntilEdit.value <= validFromEdit.value)
                            return i18n("Valid until time has to be after the valid from time.")
                        return '';
                    }
                }
            }
        }
    }
}
