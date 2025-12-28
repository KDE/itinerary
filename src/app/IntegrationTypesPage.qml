// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.itinerary as Itinerary

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title", "Integrations")

    ListView {
        currentIndex: -1
        model: Itinerary.AccountModel.accountTypeModel

        delegate: Delegates.RoundedItemDelegate {
            id: integrationDelegate

            required property int index
            required property string identifier
            required property string name
            required property string description
            required property string iconName

            text: name
            icon.name: Qt.resolvedUrl(iconName)

            contentItem: Delegates.SubtitleContentItem {
                itemDelegate: integrationDelegate
                subtitle: integrationDelegate.description
            }

            onClicked: {
                Itinerary.AccountModel.create(identifier);
            }
        }
    }
}
