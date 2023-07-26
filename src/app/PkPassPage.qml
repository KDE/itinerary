/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import org.kde.kirigami 2.17 as Kirigami
import org.kde.pkpass 1.0 as KPkPass
import "." as App

Kirigami.ScrollablePage {
    id: root
    property string passId
    property var pass
    title: {
        switch (pass.type) {
            case KPkPass.Pass.BoardingPass: return i18n("Boarding Pass");
            case KPkPass.Pass.EventTicket: return i18n("Event Ticket");
            case KPkPass.Pass.Generic: return i18n("Pass");
        }
    }

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    Component {
        id: boardingPass
        App.BoardingPass {
            passId: root.passId
            pass: root.pass
        }
    }

    Component {
        id: eventTicket
        App.EventTicket {
            passId: root.passId
            pass: root.pass
        }
    }

    Component {
        id: genericPass
        App.GenericPass {
            passId: root.passId
            pass: root.pass
        }
    }

    Item {
        id: contentItem
        width: parent.width
        implicitHeight: loader.item.implicitHeight

        Loader {
            id: loader
            x: (parent.width - implicitWidth) / 2
            sourceComponent: {
                switch (root.pass.type) {
                    case KPkPass.Pass.BoardingPass: return boardingPass;
                    case KPkPass.Pass.EventTicket: return eventTicket;
                    case KPkPass.Pass.Generic: return genericPass;
                }
            }
        }

        Connections {
            target: loader.item
            function onScanModeToggled() {
                scanModeController.toggle();
            }
        }
    }
}
