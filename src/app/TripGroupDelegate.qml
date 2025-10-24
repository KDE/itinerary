// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigami as Kirigami

import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick

Delegates.RoundedItemDelegate {
    id: delegate

    required property int index
    required property string name
    required property date begin
    required property date end
    required property string tripGroupId
    required property bool isSingleDay

    text: name

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            level: 4
            text: delegate.text
            elide: Text.ElideRight

            Layout.fillWidth: true
        }

        QQC2.Label {
            text: {
                if (delegate.isSingleDay)
                    return delegate.begin.toLocaleDateString();
                if (delegate.begin > 0)
                    return i18nc("date range", "%1 - %2", delegate.begin.toLocaleDateString(), delegate.end.toLocaleDateString());
                return i18n("Upcoming");
            }
            elide: Text.ElideRight
            opacity: 0.8

            Layout.fillWidth: true
        }
    }
}
