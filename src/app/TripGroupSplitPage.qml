// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Split Trip")

    /** The trip group to split */
    property var tripGroup

    /** Emitted when a group split has been executed. */
    signal splitDone()

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 2
        FormCard.FormTextFieldDelegate {
            id: nameEdit
            property bool manuallyChanged
            label: i18nc("@label:textbox", "New Trip name:")
            onTextEdited: nameEdit.manuallyChanged = true
            text: TripGroupManager.guessName(splitModel.selection)
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Sections")
    }

    FormCard.FormCard {
        Repeater {
            model: TripGroupSplitModel {
                id: splitModel
                reservationManager: ReservationManager
                elements: root.tripGroup.elements

                onSelectionChanged: {
                    if (!nameEdit.manuallyChanged) {
                        nameEdit.text = TripGroupManager.guessName(splitModel.selection);
                    }
                }
            }

            FormCard.FormCheckDelegate {
                checked: model.selected

                trailing: Kirigami.Icon {
                    source: model.iconName
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium
                }

                text: model.title
                description: model.subtitle
                descriptionItem {
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }
                onToggled: model.selected = checked
            }
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 2

        FormCard.FormButtonDelegate {
            icon.name: "split-symbolic"
            text: i18nc("@action:button", "Split trip")
            onClicked: {
                TripGroupManager.createGroup(splitModel.selection, nameEdit.text);
                root.splitDone();
                applicationWindow().pageStack.pop();
            }
        }
    }
}
