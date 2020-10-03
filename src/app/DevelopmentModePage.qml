/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.7 as Kirigami
import org.kde.itinerary 1.0

Kirigami.ScrollablePage {
    id: root
    title: "Development Mode"

    RowLayout {
        QQC2.Button {
            text: "Disable Development Mode"
            onClicked: {
                Settings.developmentMode = false;
                showPassiveNotification("Development mode disabled");
                applicationWindow().pageStack.pop();
            }
        }
    }
}
