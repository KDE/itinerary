// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("Split Trip")

    /** The trip group to split */
    property var tripGroup

    /** Emitted when a group split has been executed. */
    signal splitDone()

    header: ColumnLayout {
        Layout.fillWidth: true
        FormCard.FormHeader {
            title: i18n("New Trip")
        }
        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: nameEdit
                property bool manuallyChanged
                label: i18nc("@label:textbox", "Trip name:")
                onTextEdited: nameEdit.manuallyChanged = true
                text: TripGroupManager.guessName(splitModel.selection)
            }
        }
    }

    ListView {
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

        delegate: QQC2.ItemDelegate {
            highlighted: model.selected
            width: ListView.view.width
            contentItem: RowLayout {
                Kirigami.IconTitleSubtitle {
                    icon.source: model.iconName
                    title: model.title
                    subtitle: model.subtitle
                    Layout.fillWidth: true
                }
            }
            onClicked: model.selected = !model.selected
        }

        FloatingButton {
            anchors {
                right: parent.right
                rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
                bottom: parent.bottom
                bottomMargin: Kirigami.Units.largeSpacing
            }
            action: Kirigami.Action {
                icon.name: "split"
                text: i18n("Split trip")
                onTriggered: {
                    TripGroupManager.createGroup(splitModel.selection, nameEdit.text);
                    root.splitDone();
                    applicationWindow().pageStack.pop();
                }
            }
        }
    }
}
