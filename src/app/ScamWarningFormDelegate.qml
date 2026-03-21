/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

/** Scam airport warning form delegate. */
FormCard.FormTextDelegate {
    id: root
    /** The place to check for scam naming. */
    property var place
    /** The current trip id. */
    property string tripId

    text: i18n("This airport might not be quite where its name suggests it is.")

    icon.name: "dialog-warning"
    background: FormCard.FormDelegateBackground {
        control: root
        color: Kirigami.Theme.negativeBackgroundColor
    }
    textItem.wrapMode: Text.WordWrap

    visible: ScamWarningManager.warnForPlace(place, tripId)

    trailing: QQC2.ToolButton {
        display: QQC2.AbstractButton.IconOnly
        text: i18nc("@info:tooltip", "Ignore warning")
        icon.name: "dialog-close-symbolic"

        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.visible: hovered

        onClicked: ignoreConfirmDialog.open()
    }

    Kirigami.PromptDialog {
        id: ignoreConfirmDialog

        title: i18n("Ignore Warning")
        subtitle: i18n("Do you want to ignore the warning permanently or for this trip only?")
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Ignore permanently")
                icon.name: "dialog-close-symbolic"
                onTriggered: {
                    ScamWarningManager.ignorePlacePermanently(root.place);
                    root.visible = false;
                    ignoreConfirmDialog.close();
                }
            },
            Kirigami.Action {
                text: i18n("Ignore for this trip")
                icon.name: "dialog-close-symbolic"
                onTriggered: {
                    ScamWarningManager.ignorePlaceForTrip(root.place, root.tripId);
                    root.visible = false;
                    ignoreConfirmDialog.close();
                }
            }
        ]
    }
}
