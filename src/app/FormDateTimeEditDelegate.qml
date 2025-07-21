/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormDateTimeDelegate {
    id: root

    property variant obj
    property string propertyName
    value: Util.dateTimeStripTimezone(obj, propertyName)
    readonly property bool hasValue: !isNaN(value.getTime())
    property bool isModified: Util.dateTimeStripTimezone(obj, propertyName).getTime() != value.getTime()

    property date initialDay

    initialValue: {
        if (isNaN(root.initialDay.getTime())) {
            let d = new Date();
            d.setTime(d.getTime() + 60 * 60 * 1000 - (d.getTime() % (60 * 60 * 1000)));
            return d;
        }
        let d = root.initialDay;
        d.setTime(d.getTime() + 6 * 60 * 60 * 1000 - (d.getTime() % (60 * 60 * 1000)));
        return d;
    }
}
