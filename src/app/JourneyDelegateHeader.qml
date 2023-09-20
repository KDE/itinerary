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

ColumnLayout {
    id: root

    required property var journey

    //defaultColor: journey.disruptionEffect == Disruption.NormalService ? Kirigami.Theme.backgroundColor : Kirigami.Theme.negativeTextColor

    RowLayout {
        id: headerLayout

        Layout.fillWidth: true
        Layout.bottomMargin: Kirigami.Units.largeSpacing
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.leftMargin: Kirigami.Units.gridUnit
        Layout.rightMargin: Kirigami.Units.gridUnit

        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: Localizer.formatTime(journey, "scheduledDepartureTime")
            color: Kirigami.Theme.textColor
            font.strikeout: journey.disruptionEffect === Disruption.NoService
        }
        QQC2.Label {
            text: (journey.departureDelay >= 0 ? "+" : "") + journey.departureDelay;
            color: journey.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor;
            visible: journey.hasExpectedDepartureTime && journey.disruption != Disruption.NoService
            font.strikeout: journey.disruptionEffect === Disruption.NoService
        }
        QQC2.Label {
            text: "-"
            color: Kirigami.Theme.textColor
            font.strikeout: journey.disruptionEffect === Disruption.NoService
        }
        QQC2.Label {
            text: Localizer.formatTime(journey, "scheduledArrivalTime")
            color: Kirigami.Theme.textColor
            font.strikeout: journey.disruptionEffect === Disruption.NoService
        }
        QQC2.Label {
            text: (journey.arrivalDelay >= 0 ? "+" : "") + journey.arrivalDelay;
            color: journey.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor;
            visible: journey.hasExpectedArrivalTime && journey.disruption != Disruption.NoService
            font.strikeout: journey.disruptionEffect === Disruption.NoService
        }



        QQC2.Label {
            text: Localizer.formatDuration(journey.duration) + i18n(" h")
            font.bold: true
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            font.strikeout: journey.disruptionEffect === Disruption.NoService
        }
    }

     Kirigami.Separator {
         opacity: 0.5
         Layout.fillWidth: true
         Accessible.ignored: true
     }
}
