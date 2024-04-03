/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels as Models
import org.kde.kitemmodels
import org.kde.kirigami as Kirigami
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("Passes and Programs")

    actions: [
        Kirigami.Action {
            text: i18n("Add Program Membership...")
            icon.name: "list-add-symbolic"
            onTriggered: {
                const editorComponent = Qt.createComponent("org.kde.itinerary", "ProgramMembershipEditor");
                applicationWindow().pageStack.push(editorComponent, {
                    programMembership: Factory.makeProgramMembership(),
                    passId: "",
                    pageStack: pageStack
                });
            }
        }
    ]

    Models.DelegateChooser {
        id: chooser
        role: "type"
        Models.DelegateChoice {
            roleValue: PassManager.ProgramMembership
            QQC2.ItemDelegate {
                highlighted: false
                text: contentItem.title
                width: ListView.view.width
                contentItem: Kirigami.IconTitleSubtitle {
                    icon.name: "meeting-attending"
                    title: model.pass.programName
                    subtitle: {
                        if (!model.pass.member.name)
                            return model.pass.membershipNumber;
                        if (!model.pass.membershipNumber)
                            return model.pass.member.name;
                        return i18nc("name - number", "%1 - %2", model.pass.member.name, model.pass.membershipNumber)
                    }
                }
                onClicked: {
                    const programMembershipPage = Qt.createComponent("org.kde.itinerary", "ProgramMembershipPage");
                    applicationWindow().pageStack.push(programMembershipPage, {
                        programMembership: model.pass,
                        passId: model.passId,
                    });
                }
                Accessible.onPressAction: clicked()
            }
        }
        Models.DelegateChoice {
            roleValue: PassManager.PkPass
            QQC2.ItemDelegate {
                readonly property string pkPassId: PkPassManager.passId(model.pass)
                readonly property var pkPass: PkPassManager.pass(pkPassId)
                highlighted: false
                text: contentItem.title
                width: ListView.view.width
                contentItem: Kirigami.IconTitleSubtitle {
                    title: pkPass.description
                    subtitle: pkPass.organizationName
                    icon.name: pkPass.hasIcon ? "image://org.kde.pkpass/" + pkPassId + "/icon" : "bookmarks"
                }
                onClicked: {
                    const pkpassComponent = Qt.createComponent("org.kde.itinerary", "GenericPkPassPage");
                    applicationWindow().pageStack.push(pkpassComponent, {
                        passId: pkPassId,
                        pass: pkPass,
                        genericPassId: model.passId,
                    });
                }
                Accessible.onPressAction: click()
            }
        }
        Models.DelegateChoice {
            roleValue: PassManager.Ticket
            QQC2.ItemDelegate {
                id: delegate
                readonly property string pkPassId: PkPassManager.passId(model.pass)
                highlighted: false
                text: contentItem.title
                width: ListView.view.width
                contentItem: Kirigami.IconTitleSubtitle {
                    icon.name: delegate.pkPassId !== "" ? "image://org.kde.pkpass/" + delegate.pkPassId + "/icon" : "bookmarks"
                    title: model.pass.name
                    subtitle: {
                        if (model.pass.underName.name === "")
                            return model.validRangeLabel;
                        if (model.validRangeLabel === "")
                            return model.pass.underName.name;
                        return i18nc("name - valid time range", "%1 - %2", model.pass.underName.name, model.validRangeLabel);
                    }
                }
                onClicked: {
                    const ticketComponent = Qt.createComponent("org.kde.itinerary", "TicketPage");
                    applicationWindow().pageStack.push(ticketComponent, {
                        ticket: model.pass,
                        passId: model.passId,
                    });
                }
                Accessible.onPressAction: click()
            }
        }
    }

    ListView {
        id: passListView
        model: PassManager
        delegate: chooser
        section.delegate: Kirigami.ListSectionHeader {
            text: section
            width: ListView.view.width
        }
        section.property: "section"

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width
            visible: passListView.count === 0
            icon.name: "wallet-open"
            text: i18n("Import bonus or discount program cards or flat rate passes.")
            helpfulAction: Kirigami.Action {
                text: i18n("Import...")
                icon.name: "document-open"
                onTriggered: importFileDialog.open()
            }
        }
    }

}
