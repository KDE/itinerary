/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import "." as App

Kirigami.ApplicationWindow {
    title: "PkPass Viewer"
    reachableModeEnabled: false

    width: 480
    height: 720

    pageStack.initialPage: pkPassPage

    Component {
        id: pkPassPage
        App.PkPassPage {
            pass: _pass
            passId: _passId
        }
    }
}
