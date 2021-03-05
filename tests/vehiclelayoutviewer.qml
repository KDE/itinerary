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
    title: "Vehicle Layout Viewer"
    reachableModeEnabled: false

    width: 480
    height: 720

    pageStack.initialPage: layoutPage

    KPT.Manager {
        id: ptMgr;
    }

    Component {
        id: layoutPage
        App.VehicleLayoutPage {
            stopover: _stopover
            publicTransportManager: ptMgr
        }
    }
}

