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

FormCard.FormCardPage {
    id: root

    property ImportController controller

    title: i18nc("@title", "Import")

    FormCard.FormHeader {
        title: i18n("Trip")
        visible: root.controller.hasSelectedReservation
    }

    TripGroupSelectorCard {
        id: tripGroupSelector
        visible: root.controller.hasSelectedReservation
        tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(root.controller.selectionBeginDateTime, root.controller.selectionEndDateTime)
        suggestedName: root.controller.tripGroupName === "" ?
            TripGroupManager.guessNameForReservations(root.controller.selectedReservations) :
            root.controller.tripGroupName
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Items to Import")
    }

    FormCard.FormCard {
        id: stagingList

        Repeater {
            model: root.controller

            delegate: FormCard.FormCheckDelegate {
                checked: model.selected
                text: model.title
                icon.name: model.iconName
                description: model.subtitle
                trailing: ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.Icon {
                        source: "meeting-attending"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        visible: model.batchSize > 0
                    }
                    Kirigami.Icon {
                        source: "mail-attachment-symbolic"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        visible: model.attachmentCount > 0
                    }
                }
                onToggled: model.selected = checked
            }
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 2

        FormCard.FormButtonDelegate {
            icon.name: "document-open-symbolic"
            text: i18nc("@action:button", "Import selection")
            enabled: root.controller.hasSelection && (tripGroupSelector.isValidInput || !root.controller.hasSelectedReservation)
            onClicked: {
                switch (tripGroupSelector.mode) {
                    case TripGroupSelectorCard.Mode.Create:
                        root.controller.tripGroupName = tripGroupSelector.name;
                        break;
                    case TripGroupSelectorCard.Mode.Add:
                        root.controller.tripGroupId = tripGroupSelector.tripGroupId;
                }
                ApplicationController.commitImport(root.controller);
                if (root.controller.count === 0) {
                    applicationWindow().pageStack.layers.pop();
                }

                tripGroupSelector.resetEditState();
            }
        }
    }

    Component.onDestruction: root.controller.clear()
}
