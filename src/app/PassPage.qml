/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.qmlmodels 1.0 as Models
import org.kde.kirigami 2.19 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Passes and Programs")

    Component {
        id: programMembershipPage
        App.ProgramMembershipPage {}
    }

    Models.DelegateChooser {
        id: chooser
        role: "type"
        Models.DelegateChoice {
            roleValue: PassManager.ProgramMembership
            Kirigami.BasicListItem {
                highlighted: false
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
    }

    ListView {
        id: passListView
        model: PassManager
        delegate: chooser

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: passListView.count === 0
            icon.name: "wallet-open"
            text: i18n("Import bonus or discount program cards or flat rate passes.")
            helpfulAction: Kirigami.Action {
                text: i18n("Import...")
                icon.name: i18n("document-open")
                onTriggered: importDialog.open()
            }
        }
    }

}
