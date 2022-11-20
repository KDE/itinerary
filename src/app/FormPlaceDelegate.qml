// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

MobileForm.AbstractFormDelegate {
    id: root

    property alias place: internal.place
    property alias controller: internal.controller
    property alias isRangeBegin: internal.isRangeBegin
    property alias isRangeEnd: internal.isRangeEnd
    property alias showLocationName: internal.showLocationName

    background: Item {}
    Layout.fillWidth: true
    visible: place && !place.address.isEmpty
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Location")
            elide: Text.ElideRight
        }
        App.PlaceDelegate {
            id: internal
            place: place
            controller: root.controller
            isRangeBegin: true
        }
    }
}
