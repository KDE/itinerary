/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPT
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

    Component {
        id: journeySectionPage
        App.JourneySectionPage {}
    }
    Component {
        id: journeyPathPage
        App.JourneyPathPage {}
    }

    Component {
        id: jnyQueryPage
        App.JourneyQueryPage {
            title: "Journey Query"
            journeyRequest: _request
            publicTransportManager: ptMgr
        }
    }
}
