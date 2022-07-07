/*
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

import org.kde.kirigami 2.4 as Kirigami

import org.kde.kirigamiaddons.dateandtime 0.1

/**
 * A small in-line field to input a date.
 *
 * Use case is for entering a known date, not for "browsing" dates
 * where you want the day of the week.
 */
Loader {
    id: root
    property date selectedDate: new Date()

    //maybe we need something more like QQC2 combox to handle user changed signals separately from the others

    source: Kirigami.Settings.tabletMode ? "MobileDateInput.qml" : "DesktopDateInput.qml"

    onSelectedDateChanged: {
        if (item && root.selectedDate) {
            item.selectedDate = root.selectedDate;
        }
    }

    onLoaded: {
        item.selectedDate = root.selectedDate
        root.selectedDate = Qt.binding(function() {return item.selectedDate});
    }
}

