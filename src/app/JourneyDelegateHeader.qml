/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

TimelineDelegateHeaderBackground {
    property var journey

    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
    Kirigami.Theme.inherit: false
    defaultColor: journey.disruptionEffect == Disruption.NormalService ? Kirigami.Theme.backgroundColor : Kirigami.Theme.negativeTextColor
    implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2

    RowLayout {
        id: headerLayout
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: Localizer.formatTime(journey, "scheduledDepartureTime")
            color: Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: (journey.departureDelay >= 0 ? "+" : "") + journey.departureDelay;
            color: journey.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor;
            visible: journey.hasExpectedDepartureTime && journey.disruption != Disruption.NoService
        }

        QQC2.Label {
            text: Localizer.formatDuration(journey.duration)
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
        }
    }
}
