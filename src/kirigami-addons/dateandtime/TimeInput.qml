/*
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.5
import org.kde.kirigamiaddons.dateandtime 0.1

/**
 * TimeInput is a single line time editor.
 */
TextField
{
    id: timeInput

    /**
     * This property holds the desired time format.
     */
    property string format: Qt.locale().timeFormat(Locale.ShortFormat)

    /**
     * This property holds the current time value.
     */
    property date value: new Date()

    // The text field acts as a time input field.
    inputMethodHints: Qt.ImhTime

    validator: TimeInputValidator {
        id: timeValidator
        format: timeInput.format
    }

    onEditingFinished: textToValue()
    onValueChanged: valueToText()

    function textToValue() {
        const locale = Qt.locale();
        timeInput.value = Date.fromLocaleTimeString(locale, timeInput.text, timeInput.format);
    }

    function valueToText() {
        const locale = Qt.locale();
        timeInput.text = timeInput.value.toLocaleTimeString(locale, timeInput.format);
    }

    Component.onCompleted: valueToText()

    MouseArea {
        anchors.fill: parent
        enabled: Qt.platform.os === 'android'
        onClicked: AndroidIntegration.showTimePicker(timeInput.value.getTime());
        Connections {
            enabled: Qt.platform.os === 'android'
            ignoreUnknownSignals: !enabled
            target: enabled ? AndroidIntegration : null
            function onTimePickerFinished(accepted, newDate) {
                if (accepted) {
                    timeInput.value = newDate;
                }
            }
        }
    }
}
