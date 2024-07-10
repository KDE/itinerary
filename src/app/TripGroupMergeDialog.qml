// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormCardDialog {
    id: root

    property string tripGroupId
    property var tripGroup

    title: i18nc("@title:window", "Merge Trips")


    function guessName() {
        if (nameEdit.nameChanged)
            return;

        if (!root.tripGroup.automaticName) {
            nameEdit.text = root.tripGroup.name;
            return;
        }

        const otherGroup = TripGroupManager.tripGroup(tripSelector.currentValue);
        if (!otherGroup.automaticName) {
            nameEdit.text = otherGroup.name;
            return;
        }

        nameEdit.text = TripGroupManager.guessName(tripGroup.elements.concat(otherGroup.elements));
    }

    FormCard.FormComboBoxDelegate {
        id: tripSelector
        text: i18n("Merge with:")
        textRole: "name"
        valueRole: "tripGroupId"
        model: TripGroupFilterProxyModel {
            sourceModel: TripGroupModel
            filteredGroupIds: TripGroupModel.adjacentTripGroups(tripGroupId)
        }
        onCurrentValueChanged: root.guessName()
    }

    FormCard.FormTextFieldDelegate {
        id: nameEdit

        property bool nameChanged: false

        label: i18nc("@label:textbox", "Trip name:")
        onAccepted: root.accept();
        text: tripGroup?.name ?? ""

        onTextEdited: nameEdit.nameChanged = true
    }

    standardButtons: QQC2.Dialog.Cancel | QQC2.Dialog.Save

    onAccepted: () => {
        TripGroupManager.merge(root.tripGroupId, tripSelector.currentValue, nameEdit.text);
        root.close();
    }

    onTripGroupChanged: {
        tripSelector.currentIndex = 0;
        nameEdit.nameChanged = false;
        root.guessName();
    }

    Component.onCompleted: {
        let btn = standardButton(QQC2.Dialog.Save);
        btn.text = i18n("Merge");
        btn.icon.name = "merge";
    }
}
