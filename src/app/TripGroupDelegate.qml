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
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    property var tripGroup;
    property var tripGroupId;
    property var rangeType;

    id: root
    showClickFeedback: true
    topPadding: rangeType == TimelineModel.RangeEnd ? 0 : Kirigami.Units.largeSpacing

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.rightMargin: -root.rightPadding
        anchors.bottomMargin: root.rangeType == TimelineModel.RangeEnd ? -root.bottomPadding : 0

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: switch (rangeType) {
                    case TimelineModel.SelfContained: return "go-next-symbolic";
                    case TimelineModel.RangeBegin: return "go-down-symbolic";
                    case TimelineModel.RangeEnd: return "go-up-symbolic";
                }
                width: Kirigami.Units.iconSizes.smallMedium
                height: width
                color: Kirigami.Theme.textColor
                isMask: true
            }
            QQC2.Label {
                text: root.rangeType == TimelineModel.RangeEnd ? i18n("End: %1", tripGroup.name) : i18n("Trip: %1", tripGroup.name)
                color: Kirigami.Theme.textColor
                Layout.fillWidth: true
            }
        }
    }

    contentItem: RowLayout {
        id: contentLayout
        visible: root.rangeType != TimelineModel.RangeEnd

        QQC2.Label {
            text: i18np("Date: %2 (one day)", "Date: %2 (%1 days)",
                       Math.ceil((root.tripGroup.endDateTime.getTime() - root.tripGroup.beginDateTime.getTime()) / (1000 * 3600 * 24)),
                       Localizer.formatDateTime(root.tripGroup, "beginDateTime"))
        }

        Component.onCompleted: {
            // hide content entirely in the header-only end elements
            parent.visible = contentLayout.visible
        }
    }

    onClicked: {
        if (rangeType == TimelineModel.SelfContained)
            _timelineModel.expand(tripGroupId);
        else
            _timelineModel.collapse(tripGroupId);
    }
}
