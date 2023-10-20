// SPDX-FileCopyrightText: 2018,2023 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.itinerary 1.0
import "." as App

FormCard.AbstractFormDelegate {
    id: root

    property var stopover
    property string sections
    property string scheduledPlatform

    text: i18n("Platform")

    visible: platformLabel.text != ""

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: root.text
            elide: Text.ElideRight
            Layout.fillWidth: true
            Accessible.ignored: true
        }

        RowLayout {
            QQC2.Label {
                Layout.fillWidth: true
                id: platformLabel
                text: (root.stopover.hasExpectedPlatform ? root.stopover.expectedPlatform : root.scheduledPlatform) + (root.sections ? " " + root.sections: "")
                color: root.platformChanged ? Kirigami.Theme.negativeTextColor :
                    root.stopover.hasExpectedPlatform ? Kirigami.Theme.positiveTextColor :
                    Kirigami.Theme.disabledTextColor
            }

            QQC2.Label {
                text: i18nc("previous platform", "(was: %1)", root.scheduledPlatform)
                visible: root.stopover.platformChanged && root.scheduledPlatform !== ""
                color: Kirigami.Theme.disabledTextColor
                Accessible.ignored: !visible
            }
        }
    }
}
