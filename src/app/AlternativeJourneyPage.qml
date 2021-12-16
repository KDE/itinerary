/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.JourneyQueryPage {
    id: root

    property QtObject controller;

    title: i18n("Alternative Connections")
    journeyRequest: controller.journeyRequest

    onJourneyChanged: replaceWarningSheet.sheetOpen = true

    Kirigami.OverlaySheet {
        id: replaceWarningSheet

        header: Kirigami.Heading {
            text: i18n("Replace Journey")
        }

        QQC2.Label {
            text: i18n("Do you really want to replace your existing reservation with the newly selected journey?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Replace")
                icon.name: "document-save"
                onClicked: {
                    controller.applyJourney(root.journey);
                    applicationWindow().pageStack.pop();
                }
            }
        }
    }
}
