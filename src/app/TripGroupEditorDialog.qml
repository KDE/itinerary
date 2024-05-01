// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormCardDialog {
    id: root

    property int mode
    property var tripGroup

    enum Mode {
        Edit,
        Add
    }

    title: mode === TripGroupEditorDialog.Mode.Add ? i18nc("@title:window", "Add Trip") : i18nc("@title:window", "Edit Trip")

    FormCard.FormTextFieldDelegate {
        label: i18nc("@label:textbox", "Trip name:")
        onAccepted: buttonBox.accepted();
        text: tripGroup.name
    }

    standardButtons: QQC2.Dialog.Cancel | QQC2.Dialog.Save

    onAccepted: () => {
        root.close();
    }
}
