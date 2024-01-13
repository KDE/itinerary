/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    required property string passId
    required property var programMembership
    required property var pageStack

    readonly property bool isValidInput: programNameEdit.text !== '' && (!validFromEdit.hasValue || !validUntilEdit.hasValue || validFromEdit.value < validUntilEdit.value)

    title: i18n("Edit Program Membership")

    data: FloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: Kirigami.Action {
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
                root.pageStack.pop();
            }
        }
    }

    CardPageTitle {
        emojiIcon: "🎫"
        text: i18n("Program")

        Layout.fillWidth: true
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: programNameEdit
            label: i18n("Name")
            text: programMembership.programName
            status: Kirigami.MessageType.Error
            statusMessage: text === "" ? i18n("Program name must not be empty.") : ""
        }
    }

    FormCard.FormHeader {
        title: i18n("Membership")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: memberNameEdit
            label: i18n("Member")
            text: programMembership.member.name
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: numberEdit
            label: i18n("Number")
            text: programMembership.membershipNumber
        }
        FormCard.FormDelegateSeparator {}
        // TODO date-only edit delegates
        FormDateTimeEditDelegate {
            id: validFromEdit
            text: i18n("Valid from")
            obj: root.programMembership
            propertyName: "validFrom"
        }
        FormCard.FormDelegateSeparator {}
        FormDateTimeEditDelegate {
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
