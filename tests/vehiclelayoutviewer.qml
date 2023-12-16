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
            selectedVehicleSection: _coach
            seat: _seat
            publicTransportManager: ptMgr
        }
    }
}

