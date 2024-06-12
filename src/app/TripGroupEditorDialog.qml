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

    property string tripGroupId
    property var tripGroup

    title: root.tripGroupId === "" ? i18nc("@title:window", "Add Trip") : i18nc("@title:window", "Edit Trip")

    FormCard.FormTextFieldDelegate {
        id: nameEdit
        label: i18nc("@label:textbox", "Trip name:")
        onAccepted: root.accept();
        text: tripGroup?.name ?? ""
    }

    standardButtons: QQC2.Dialog.Cancel | QQC2.Dialog.Save

    onAccepted: () => {
        if (root.tripGroup.name !== nameEdit.text) {
            root.tripGroup.name = nameEdit.text;
            root.tripGroup.automaticName = false;
        }
        root.close();
    }

    onTripGroupChanged: {
        // needs to be done explicitly as editing the text breaks the binding
        // and thus doesn't work when reusing the dialog a second time
        nameEdit.text = root.tripGroup.name;
    }
}
