// SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.3
import QtQuick.Window 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.8 as Kirigami

/**
 * A popup that prompts the user to select a date
 */
Kirigami.OverlaySheet {
    id: root
    /**
     * The year of the selected date
     */
    property alias year: datePicker.year
    /**
     * The month of the selected date
     */
    property alias month: datePicker.month
    /**
     * The selected date
     */
    property alias selectedDate: datePicker.selectedDate

    /**
     * Emitted when the user accepts the dialog.
     * The selected date is available from the selectedDate property.
     */
    signal accepted()

    /**
     * Emitted when the user cancells the popup
     */
    signal cancelled()

    header: RowLayout {
        implicitWidth: datePicker.width
        Kirigami.Heading {
            level: 2
            text: datePicker.selectedDate.toLocaleDateString(Qt.locale(), "<b>MMMM</b>")
        }
        Kirigami.Heading {
            level: 3
            text: datePicker.selectedDate.getFullYear()
            opacity: 0.8
            Layout.fillWidth: true
        }

        Controls.Button {
            icon.name: "go-previous"
            Controls.ToolTip.text: i18n("Previous")
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: Kirigami.Units.shortDuration
            onClicked: datePicker.prevMonth()
        }
        Controls.Button {
            icon.name: "go-jump-today"
            Controls.ToolTip.text: i18n("Today")
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: Kirigami.Units.shortDuration
            onClicked: datePicker.goToday()
        }
        Controls.Button {
            icon.name: "go-next"
            Controls.ToolTip.text: i18n("Next")
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.delay: Kirigami.Units.shortDuration
            onClicked: datePicker.nextMonth()
        }
    }
    contentItem: DatePicker {
        id: datePicker
        implicitWidth: Math.min(Window.width, Kirigami.Units.gridUnit * 25)
        implicitHeight: width * 0.8
    }
    footer: RowLayout {
        Controls.Label {
            text: datePicker.selectedDate.toLocaleDateString();
            Layout.fillWidth: true
        }
        Controls.Button {
            text: i18n("Cancel")
            icon.name: "dialog-cancel"
            onClicked: {
                root.cancelled()
                root.close()
            }

        }
        Controls.Button {
            text: i18n("Accept")
            icon.name: "dialog-ok-apply"
            onClicked: {
                root.selectedDate = datePicker.selectedDate
                root.accepted()
                root.close()
            }
        }
    }
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
}
