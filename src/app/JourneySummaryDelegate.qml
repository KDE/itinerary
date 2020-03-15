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

RowLayout {
    property var journey

    Repeater {
        model: journey.sections
        delegate: Kirigami.Icon {
            source: {
                switch (modelData.mode) {
                    case JourneySection.PublicTransport:
                        return PublicTransport.lineIcon(modelData.route.line);
                    case JourneySection.Walking: return "qrc:///images/walk.svg";
                    case JourneySection.Waiting: return "qrc:///images/wait.svg";
                    case JourneySection.Transfer: return "qrc:///images/transfer.svg";
                        default: return "question";
                }
            }
            color: PublicTransport.warnAboutSection(modelData) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            width: isMask ? height : implicitWidth
            height: Kirigami.Units.iconSizes.small
            isMask: modelData.mode != JourneySection.PublicTransport || !modelData.route.line.hasLogo
            Layout.preferredWidth: width
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
        }
    }
    QQC2.Label {
        text: i18np("One change", "%1 changes", journey.numberOfChanges)
    }
}
