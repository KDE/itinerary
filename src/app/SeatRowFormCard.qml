// SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb-mail@gmx.de>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCard {
    id: root

    default property alias __rowContent: innerLayout.data

    FormCard.AbstractFormDelegate {
        background: null
        contentItem: RowLayout {
            id: innerLayout

            // not enough space for all labels even with giving up equal sizing, so hide low-priority content
            readonly property bool hideLowPriorityContent: {
                let width = 0;
                for (const child of innerLayout.children) {
                    width += child.implicitWidth + 2 * Kirigami.Units.smallSpacing;
                }
                return width > innerLayout.width;
            }

            // not enough space with optional content gone either, so enable eliding as a last resort
            readonly property bool enableEliding: {
                let width = 0;
                for (const child of innerLayout.children) {
                    if (!child.hasOwnProperty("lowPriority") || !child.lowPriority) {
                        width += child.implicitWidth + 2 * Kirigami.Units.smallSpacing;
                    }
                }
                return width > innerLayout.width;
            }

            spacing: 0
        }
    }
}

