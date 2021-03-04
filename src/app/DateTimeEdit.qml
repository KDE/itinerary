/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

MouseArea {
    id: root

    property variant obj
    property string propertyName
    property date value: Util.dateTimeStripTimezone(obj, propertyName)
    property bool isModified: Util.dateTimeStripTimezone(obj, propertyName).getTime() != value.getTime()

    implicitHeight: layout.implicitHeight
    implicitWidth: layout.implicitWidth

    RowLayout {
        id: layout

        QQC2.Label {
            text: isModified ? value.toLocaleString(Qt.locale(), Locale.ShortFormat) : Localizer.formatDateTime(obj, propertyName)
        }

        Kirigami.Icon {
            source: "document-edit"
            width: Kirigami.Units.iconSizes.smallMedium
            height: width
        }
    }

    // FIXME super ugly, we reference stuff from our parent here
    onClicked: {
        dateTimeEditSheet.value = isNaN(root.value.getTime()) ? new Date() : root.value;
        dateTimeEditSheet.sheetOpen = true;
        conn.enabled = true;
    }

    Connections {
        id: conn
        enabled: false
        target: dateTimeEditSheet
        onSheetOpenChanged: {
            console.log(root.value, dateTimeEditSheet.value);
            root.value = dateTimeEditSheet.value;
            root.value.setSeconds(0, 0);
            enabled = false;
        }
    }
}
