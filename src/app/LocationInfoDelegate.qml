/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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
import org.kde.kitinerary 1.0 as KItinerary
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    id: root
    property var locationInfo;

   header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.rightMargin: -root.rightPadding

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "documentinfo"
                width: Kirigami.Units.iconSizes.small
                height: width
                color: Kirigami.Theme.neutralTextColor
                isMask: true
            }
            QQC2.Label {
                text: i18n("Entering %1", Localizer.countryName(locationInfo.isoCode))
                color: Kirigami.Theme.neutralTextColor
                Layout.fillWidth: true
            }
        }
    }

    contentItem: ColumnLayout {
        id: topLayout

        QQC2.Label {
            Layout.fillWidth: true
            text: locationInfo.drivingSide == KItinerary.KnowledgeDb.DrivingSide.Right ?
                i18n("People are driving on the right side.") :
                i18n("People are driving on the wrong side.")
            color: Kirigami.Theme.negativeTextColor
            visible: locationInfo.drivingSideDiffers
            wrapMode: Text.WordWrap
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("No compatible power sockets: %1", locationInfo.powerSocketTypes)
            color: Kirigami.Theme.negativeTextColor
            visible: locationInfo.powerPlugCompatibility == LocationInformation.Incompatible
            wrapMode: Text.WordWrap
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Some incompatible power sockets: %1", locationInfo.powerSocketTypes)
            color: Kirigami.Theme.neutralTextColor
            visible: locationInfo.powerPlugCompatibility == LocationInformation.PartiallyCompatible && locationInfo.powerSocketTypes != ""
            wrapMode: Text.WordWrap
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Some incompatible power plugs: %1", locationInfo.powerPlugTypes)
            color: Kirigami.Theme.neutralTextColor
            visible: locationInfo.powerPlugCompatibility == LocationInformation.PartiallyCompatible && locationInfo.powerPlugTypes != ""
            wrapMode: Text.WordWrap
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Timezone change: %1", locationInfo.timeZoneName)
            color: Kirigami.Theme.neutralTextColor
            visible: locationInfo.timeZoneDiffers
            wrapMode: Text.WordWrap
        }
    }
}

