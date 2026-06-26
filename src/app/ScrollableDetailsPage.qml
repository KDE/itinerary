// SPDX-FileCopyrightText: 2026 Jonah Brüchert <jbb@kaidan.im>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

DetailsPage {
    default property alias content: layout.children

    padding: 0

    QQC2.ScrollView {
        id: scrollable

        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: contentWidth

            Item {
                implicitHeight: Kirigami.Units.gridUnit
            }

            ColumnLayout {
                id: layout

                Layout.fillWidth: true
            }

            Item {
                implicitHeight: 20
            }
        }
    }
}
