/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Rectangle {
    property var journey

    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
    Kirigami.Theme.inherit: false
    color: journey.disruptionEffect == Disruption.NormalService ? Kirigami.Theme.backgroundColor : Kirigami.Theme.negativeTextColor
    implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
    anchors.leftMargin: -root.leftPadding
    anchors.topMargin: -root.topPadding
    anchors.rightMargin: -root.rightPadding

    RowLayout {
        id: headerLayout
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: Localizer.formatTime(journey, "scheduledDepartureTime")
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: Localizer.formatDuration(journey.duration)
            color: Kirigami.Theme.textColor
        }
    }
}
