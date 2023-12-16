/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import "." as App

Kirigami.ApplicationWindow {
    title: "PkPass Viewer"

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
