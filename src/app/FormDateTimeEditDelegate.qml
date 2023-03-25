/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.dateandtime 0.1 as Addon
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

MobileForm.AbstractFormDelegate {
    id: root

    property variant obj
    property string propertyName
    property date value: Util.dateTimeStripTimezone(obj, propertyName)
    readonly property bool hasValue: !isNaN(value.getTime())
    property bool isModified: Util.dateTimeStripTimezone(obj, propertyName).getTime() != value.getTime()

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        QQC2.Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: root.text
            Accessible.ignored: true
        }
        Addon.DateInput {
            id: dateInput
            selectedDate: root.value
            onSelectedDateChanged: root.updateValue()
            visible: root.hasValue
        }
        Addon.TimeInput {
            id: timeInput
            value: root.value
            onValueChanged: root.updateValue()
            visible: root.hasValue
        }
        QQC2.ToolButton {
            icon.name: "document-edit"
            visible: !root.hasValue
            onClicked: {
                dateInput.item.selectedDate = new Date();
                timeInput.value = new Date();
            }
        }
    }

    Connections {
        target: dateInput
        onSelectedDateChanged: root.updateValue()
    }
    function updateValue() {
        const MSECS_PER_DAY = 24 * 60 * 60 * 1000;
        let dt = new Date();
        dt.setTime(dateInput.selectedDate.getTime() - (dateInput.selectedDate.getTime() % MSECS_PER_DAY) + (timeInput.value.getTime() % MSECS_PER_DAY));
        dt.setSeconds(0, 0);
        console.log(dt, dateInput.selectedDate, timeInput.value);
        root.value = dt;
    }
}
