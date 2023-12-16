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
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Passes and Programs")

    Component {
        id: pkpassComponent
        App.GenericPkPassPage {}
    }
    Component {
        id: ticketComponent
        App.TicketPage {}
    }

    actions: [
        Kirigami.Action {
            text: i18n("Add Program Membership...")
            icon.name: "list-add-symbolic"
            onTriggered: applicationWindow().pageStack.push(programMembershipEditor, { programMembership: Factory.makeProgramMembership(), passId: "" })
        }
    ]

    Models.DelegateChooser {
        id: chooser
        role: "type"
        Models.DelegateChoice {
            roleValue: PassManager.ProgramMembership
            Kirigami.BasicListItem {
                highlighted: false
                icon: "meeting-attending"
                text: model.pass.programName
                subtitle: {
                    if (!model.pass.member.name)
                        return model.pass.membershipNumber;
                    if (!model.pass.membershipNumber)
                        return model.pass.member.name;
                    return i18nc("name - number", "%1 - %2", model.pass.member.name, model.pass.membershipNumber)
                }
                onClicked: applicationWindow().pageStack.push(programMembershipPage, { programMembership: model.pass, passId: model.passId })
            }
        }
        Models.DelegateChoice {
            roleValue: PassManager.PkPass
            Kirigami.BasicListItem {
                readonly property string pkPassId: PkPassManager.passId(model.pass)
                readonly property var pkPass: PkPassManager.pass(pkPassId)
                highlighted: false
                text: pkPass.description
                subtitle: pkPass.organizationName
                icon: pkPass.hasIcon ? "image://org.kde.pkpass/" + pkPassId + "/icon" : "bookmarks"
                reserveSpaceForIcon: true
                onClicked: applicationWindow().pageStack.push(pkpassComponent, { passId: pkPassId, pass: pkPass, genericPassId: model.passId });
            }
        }
        Models.DelegateChoice {
            roleValue: PassManager.Ticket
            Kirigami.BasicListItem {
                highlighted: false
                icon: "bookmarks"
                text: model.pass.name
                subtitle: {
                    if (model.pass.underName.name === "")
                        return model.validRangeLabel;
                    if (model.validRangeLabel === "")
                        return model.pass.underName.name;
                    return i18nc("name - valid time range", "%1 - %2", model.pass.underName.name, model.validRangeLabel);
                }
                onClicked: applicationWindow().pageStack.push(ticketComponent, { ticket: model.pass, passId: model.passId })
            }
        }
    }

    ListView {
        id: passListView
        model: PassManager
        delegate: chooser
        section.delegate: Kirigami.ListSectionHeader {
            text: section
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
                onTriggered: importDialog.open()
            }
        }
    }

}
