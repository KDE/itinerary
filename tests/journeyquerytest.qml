/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kpublictransport 1.0 as KPT
import "." as App

Kirigami.ApplicationWindow {
    title: "Journey Query Test"
    reachableModeEnabled: false

    width: 480
    height: 720

    pageStack.initialPage: jnyQueryPage

    KPT.Manager {
        id: ptMgr;
        allowInsecureBackends: true
    }

    QtObject {
        id: dummyController
        property var journeyRequest: _request
    }

    Component {
        id: journeySectionPage
        App.JourneySectionPage {}
    }

    Component {
        id: jnyQueryPage
        App.JourneyQueryPage {
            controller: dummyController
            publicTransportManager: ptMgr
        }
    }
}
