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
import "./components" as Components

FormCard.FormCardPage {
    id: root

    property ImportController controller

    title: i18nc("@title", "Import")

    FormCard.FormHeader {
        title: i18n("Trip")
        visible: root.controller.hasSelectedReservation && Settings.developmentMode
    }

    FormCard.FormCard {
        visible: root.controller.hasSelectedReservation && Settings.developmentMode
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                Components.RadioSelector{
                    id: newOrAddSelector
                    defaultIndex: TripGroupModel.intersectingTripGroups(root.controller.selectionBeginDateTime, root.controller.selectionEndDateTime).length == 1 ? 1 : 0

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 20
                    consistentWidth: true

                    actions: [
                        Kirigami.Action {
                            text: i18n("New Trip")
                        },
                        Kirigami.Action {
                            text: i18n("Add to Trip")
                        }
                    ]
                }
            }
        }
        Component.onCompleted: console.log(newOrAddSelector.defaultIndex, newOrAddSelector.selectedIndex)

        FormCard.FormTextFieldDelegate {
            id: nameEdit
            label: i18nc("@label:textbox", "Trip name:")
            text: root.controller.tripGroupName === "" ?
                TripGroupManager.guessNameForReservations(root.controller.selectedReservations) :
                root.controller.tripGroupName
            visible: newOrAddSelector.selectedIndex === 0
            onTextEdited: nameChangeConnection.enabled = false

            Connections {
                id: nameChangeConnection
                enabled: root.controller.tripGroupName === ""
                target: root.controller
                function onSelectionChanged() {
                    nameEdit.text = TripGroupManager.guessNameForReservations(root.controller.selectedReservations);
                }
            }
        }

        FormCard.FormComboBoxDelegate {
            id: tripGroupSelector
            text: i18n("Add to:")
            textRole: "name"
            valueRole: "tripGroupId"
            visible: newOrAddSelector.selectedIndex === 1
            model: TripGroupFilterProxyModel {
                sourceModel: TripGroupModel
                filteredGroupIds: TripGroupModel.adjacentTripGroups(root.controller.selectionBeginDateTime, root.controller.selectionEndDateTime)
            }

            Connections {
                id: groupChangeConnection
                target: root.controller
                function onSelectionChanged() {
                    if (tripGroupSelector.currentIndex >= 0)
                        return;
                    const tgIds = TripGroupModel.intersectingTripGroups(root.controller.selectionBeginDateTime, root.controller.selectionEndDateTime)
                    if (tgIds.length != 1)
                        return;
                    tripGroupSelector.currentIndex = tripGroupSelector.indexOfValue(tgIds[0]);
                }
            }

            Component.onCompleted: groupChangeConnection.onSelectionChanged()
        }
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
            enabled: root.controller.hasSelection && (
                (newOrAddSelector.selectedIndex === 0 && nameEdit.text !== "") ||
                (newOrAddSelector.selectedIndex === 1 && tripGroupSelector.currentIndex >= 0) ||
                !root.controller.hasSelectedReservation
            )
            onClicked: {
                switch (newOrAddSelector.selectedIndex) {
                    case 0:
                        root.controller.tripGroupName = nameEdit.text;
                        break;
                    case 1:
                        root.controller.tripGroupId = tripGroupSelector.currentValue;
                }
                if (!Settings.developmentMode) {
                    root.controller.tripGroupName = "";
                    root.controller.tripGroupId = "";
                }
                ApplicationController.commitImport(root.controller);
                if (root.controller.count === 0) {
                    applicationWindow().pageStack.layers.pop();
                }
            }
        }
    }

    Component.onDestruction: root.controller.clear()
}
