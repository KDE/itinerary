/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

/** New or existing trip group selection UI for import/edit pages. */
FormCard.FormCard {
    id: root

    enum Mode {
        Create,
        Add
    }
    readonly property int mode: newOrAddSelector.selectedIndex === 0 ? TripGroupSelectorCard.Mode.Create : TripGroupSelectorCard.Mode.Add

    /** Suggested name for a new trip */
    property string suggestedName: ""
    onSuggestedNameChanged: {
        if (!nameEdit.nameEdited)
            nameEdit.text = root.suggestedName;
    }

    /** Chosen name for a new trip group. */
    readonly property alias name: nameEdit.text

    /** Possible existing trips to offer as a choice */
    property list<string> tripGroupCandidates: []
    onTripGroupCandidatesChanged: tripGroupSelector.setDefaultSelection()

    /** Chosen group to add to. */
    readonly property alias tripGroupId: tripGroupSelector.currentValue

    /** Resets any state related to existing user input, for a fresh start. */
    function resetEditState() {
        nameEdit.nameEdited = false;
        nameEdit.text = Qt.binding(() => { nameEdit.text = root.suggestedName; });
        tripGroupSelector.currentIndex = -1;
        tripGroupSelector.setDefaultSelection();
    }

    /** @c false if this has invalid input the user still has to fix. */
    readonly property bool isValidInput: (root.mode === TripGroupSelectorCard.Mode.Create && root.name !== "")
        || (root.mode === TripGroupSelectorCard.Mode.Add && tripGroupSelector.currentIndex >= 0)

    FormCard.FormRadioSelectorDelegate {
        id: newOrAddSelector

        selectedIndex: root.tripGroupCandidates.length === 1
            || TripGroupModel.emptyTripGroups().length > 0
            || ApplicationController.contextTripGroupId !== "" ? 1 : 0

        consistentWidth: true

        actions: [
            Kirigami.Action {
                text: i18n("New Trip")
            },
            Kirigami.Action {
                text: i18n("Add to Trip")
                enabled: tripGroupSelector.count > 0
            }
        ]
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: nameEdit
        property bool nameEdited: false
        label: i18nc("@label:textbox", "Trip name:")
        text: root.suggestedName
        visible: newOrAddSelector.selectedIndex === 0
        onTextEdited: nameEdit.nameEdited = true

        status: Kirigami.MessageType.Error
        statusMessage: nameEdit.text === "" ? i18n("Trip name must not be empty.") : ""
    }

    FormCard.FormComboBoxDelegate {
        id: tripGroupSelector
        text: i18n("Add to:")
        textRole: "name"
        valueRole: "tripGroupId"
        visible: newOrAddSelector.selectedIndex === 1
        model: TripGroupFilterProxyModel {
            sourceModel: TripGroupModel
            filteredGroupIds: root.tripGroupCandidates
                .concat(TripGroupModel.emptyTripGroups())
                .concat([ApplicationController.contextTripGroupId])
        }
        onCountChanged: tripGroupSelector.setDefaultSelection()

        function setDefaultSelection() {
            if (tripGroupSelector.currentIndex >= 0) {
                return;
            }
            if (root.tripGroupCandidates.length == 1) {
                tripGroupSelector.currentIndex = tripGroupSelector.indexOfValue(root.tripGroupCandidates[0]);
                return;
            }
            if (ApplicationController.contextTripGroupId !== "" && (root.tripGroupCandidates.length == 0 || root.tripGroupCandidates.includes(ApplicationController.contextTripGroupId) || TripGroupModel.emptyTripGroups().includes(ApplicationController.contextTripGroupId))) {
                tripGroupSelector.currentIndex = tripGroupSelector.indexOfValue(ApplicationController.contextTripGroupId);
                return;
            }
        }

        status: Kirigami.MessageType.Error
        statusMessage: tripGroupSelector.currentIndex < 0 ? i18n("A trip must be selected.") : ""

        Component.onCompleted: tripGroupSelector.setDefaultSelection()
    }
}
