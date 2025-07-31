// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

/** Intermediate stops view toggle button for use in in timeline delegates. */
RowLayout {
    id: root

    /** Journey section to display. */
    property JourneySectionModel sectionModel
    /** Expansion state of the intermediate stops view. */
    property bool expanded
    /** Expansion state toggled. */
    signal toggled()

    Layout.fillWidth: true
    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
    QQC2.ToolButton {
        visible: root.sectionModel.sectionCount !== 0
        Layout.fillWidth: true
        onClicked: root.toggled()
        contentItem: RowLayout {
            spacing: 0
            Kirigami.Icon {
                source: root.expanded ? "arrow-up" : "arrow-down"
                implicitHeight: Kirigami.Units.largeSpacing * 2
                color: Kirigami.Theme.disabledTextColor
            }
            QQC2.Label {
                text: i18np("1 intermediate stop (%2)", "%1 intermediate stops (%2)", root.sectionModel.effectiveStopCount, Localizer.formatDuration(root.sectionModel.journeySection.duration))
                elide: Text.ElideRight
                color: Kirigami.Theme.disabledTextColor
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.fillWidth: true
            }
        }
    }
    QQC2.Label {
        visible: root.sectionModel.sectionCount === 0 && root.sectionModel.journeySection.duration > 0
        text: i18n("0 intermediate stop (%1)", Localizer.formatDuration(root.sectionModel.journeySection.duration))
        elide: Text.ElideRight
        color: Kirigami.Theme.disabledTextColor
        Layout.fillWidth: true
    }
}
