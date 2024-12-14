/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kitemmodels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    title: i18n("Passes and Programs")

    FormCard.FormHeader {
        title: i18nc("@title", "Add Program Membership")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Import From File")
            icon.name: "document-import-symbolic"
            onClicked: importFileDialog.open()
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Add Manually")
            icon.name: "list-add-symbolic"
            onClicked: {
                const editorComponent = Qt.createComponent("org.kde.itinerary", "ProgramMembershipEditor");
                root.QQC2.ApplicationWindow.window.pageStack.push(editorComponent, {
                    programMembership: Factory.makeProgramMembership(),
                    passId: "",
                    pageStack: pageStack
                });
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("not yet expired tickets", "Valid")
        visible: validRepeater.count > 0
    }

    FormCard.FormCard {
        visible: validRepeater.count > 0
        Repeater {
            id: validRepeater

            model: KSortFilterProxyModel {
                sourceModel: PassManager
                filterRoleName: "state"
                filterString: "valid"
            }

            delegate: PassDelegate {}
        }
    }

    FormCard.FormHeader {
        visible: futureRepeater.count > 0
        title: i18nc("not yet valid tickets", "Future")
    }

    FormCard.FormCard {
        visible: futureRepeater.count > 0
        Repeater {
            id: futureRepeater

            model: KSortFilterProxyModel {
                sourceModel: PassManager
                filterRoleName: "state"
                filterRowCallback: function(source_row, source_parent) {
                  return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PassManager.StateRole) === "future";
                };
            }

            delegate: PassDelegate {}
        }
    }

    FormCard.FormHeader {
        visible: expiredRepeater.count > 0
        title: i18nc("no longer valid tickets", "Expired")
    }

    FormCard.FormCard {
        visible: expiredRepeater.count > 0
        Repeater {
            id: expiredRepeater

            model: KSortFilterProxyModel {
                sourceModel: PassManager
                filterRoleName: "state"
                filterRowCallback: function(source_row, source_parent) {
                  return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PassManager.StateRole) === "expired";
                };
            }

            delegate: PassDelegate {}
        }
    }
}
