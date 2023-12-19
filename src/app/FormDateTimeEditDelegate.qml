/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

FormCard.FormDateTimeDelegate {
    id: root

    property variant obj
    property string propertyName
    value: Util.dateTimeStripTimezone(obj, propertyName)
    readonly property bool hasValue: !isNaN(value.getTime())
    property bool isModified: Util.dateTimeStripTimezone(obj, propertyName).getTime() != value.getTime()
    // TODO this is not used with KF6 yet!
    initialValue: {
        let d = new Date();
        d.setTime(d.getTime() + 60 * 60 * 1000 - (d.getTime() % (60 * 60 * 1000)));
        return d;
    }
}
