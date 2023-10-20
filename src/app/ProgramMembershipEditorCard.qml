// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitemmodels 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

ColumnLayout {
    id: root

    /** The object to edit. */
    property var item

    /** Apply current edit state to @p item. */
    function apply(item) {
        if (root.visible && programComboBox.currentIndex >= 0) {
            console.log("saving program!");
            item.programMembershipUsed = programComboBox.currentValue;
            console.log(item.programMembershipUsed);
            console.log(programComboBox.currentValue);
        }
    }

    visible: programModel.count > 0
    spacing: 0

    FormCard.FormHeader {
        title: i18nc("bonus, discount or frequent traveler program", "Program")
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: programComboBox
            model: KSortFilterProxyModel {
                id: programModel
                filterRowCallback: function(source_row) {
                    const type = sourceModel.data(sourceModel.index(source_row, 0), PassManager.PassTypeRole);
                    const validUntil = sourceModel.data(sourceModel.index(source_row, 0), PassManager.ValidUntilRole);
                    return type == PassManager.ProgramMembership && (isNaN(validUntil) || validUntil > new Date());
                }
                sourceModel: PassManager
            }
            textRole: "name"
            valueRole: "pass"
        }
        FormCard.FormDelegateSeparator { visible: programNameLabel.visible }
        FormCard.FormTextDelegate {
            id: programNameLabel
            text: i18n("Name")
            description: programComboBox.currentValue.programName
            visible: description
        }
        FormCard.FormDelegateSeparator { visible: membershipNumberLabel.visible }
        FormCard.FormTextDelegate {
            id: membershipNumberLabel
            text: i18n("Number")
            description: programComboBox.currentValue.membershipNumber
            visible: description
        }
    }

    Component.onCompleted: {
        const passId = PassManager.findMatchingPass(item.programMembershipUsed);
        if (passId === "")
            return;
        for (let i = 0; i < programModel.rowCount(); ++i) {
            console.log(i, programModel.data(programModel.index(i, 0), PassManager.PassIdRole). passId);
            if (programModel.data(programModel.index(i, 0), PassManager.PassIdRole) == passId) {
                programComboBox.currentIndex = i;
                return;
            }
        }
    }
}
