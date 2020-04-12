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

Kirigami.AbstractListItem {
    highlighted: false
    GridLayout {
        columns: 2

        // top row: departure time, departure location, departure platform
        RowLayout {
            visible: modelData.mode != JourneySection.Waiting
            QQC2.Label {
                text: Localizer.formatTime(modelData, "scheduledDepartureTime")
            }
            QQC2.Label {
                text: {
                    if (modelData.disruption == Disruption.NoService)
                        return i18n("Cancelled");
                    return (modelData.departureDelay >= 0 ? "+" : "") + modelData.departureDelay;
                }
                color: {
                    if (modelData.departureDelay > 1 || modelData.disruption == Disruption.NoService)
                        return Kirigami.Theme.negativeTextColor;
                    return Kirigami.Theme.positiveTextColor;
                }
                visible: modelData.hasExpectedDepartureTime || modelData.disruption == Disruption.NoService
            }
        }
        RowLayout {
            visible: modelData.mode != JourneySection.Waiting
            QQC2.Label {
                text: modelData.from.name
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            QQC2.Label {
                text: modelData.hasExpectedDeparturePlatform ? modelData.expectedDeparturePlatform : modelData.scheduledDeparturePlatform
                color: modelData.departurePlatformChanged ? Kirigami.Theme.negativeTextColor
                    : modelData.hasExpectedDeparturePlatform ? Kirigami.Theme.positiveTextColor
                    : Kirigami.Theme.textColor
                visible: modelData.scheduledDeparturePlatform !== ""
            }
        }

        // middle row: mode symbol, transport mode, duration
        Rectangle {
            color: (modelData.route.line.hasColor && !modelData.route.line.hasLogo && !modelData.route.line.hasModeLogo) ? modelData.route.line.color : "transparent"
            implicitHeight: Kirigami.Units.iconSizes.smallMedium
            implicitWidth: modeIcon.width
            Layout.alignment: Qt.AlignHCenter

            Kirigami.Icon {
                id: modeIcon
                anchors.centerIn: parent
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
                color: modelData.route.line.hasTextColor ? modelData.route.line.textColor : Kirigami.Theme.textColor
                width: (modelData.mode == JourneySection.PublicTransport && modelData.route.line.hasLogo && modeDAta.route.line.hasModeLogo) ? implicitWidth : height
                height: parent.height
                isMask: modelData.mode != JourneySection.PublicTransport || !modelData.route.line.hasLogo || !modelData.route.line.hasModeLogo
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: {
                switch (modelData.mode) {
                case JourneySection.PublicTransport:
                {
                    var l = modelData.route.line.modeString + " " + modelData.route.line.name;
                    if (modelData.route.direction)
                        return i18n("%1 to %2 (%3)", l, modelData.route.direction, Localizer.formatDuration(modelData.duration));
                    return i18n("%1 (%2)", l, Localizer.formatDuration(modelData.duration));
                }
                case JourneySection.Walking:
                    if (modelData.distance == 0)
                        return i18n("Walk (%1)", Localizer.formatDuration(modelData.duration));
                    return i18n("Walk %1 (%2)", Localizer.formatDistance(modelData.distance), Localizer.formatDuration(modelData.duration));
                case JourneySection.Transfer:
                    return i18n("Transfer (%1)", Localizer.formatDuration(modelData.duration))
                case JourneySection.Waiting:
                    return i18n("Wait (%1)", Localizer.formatDuration(modelData.duration))
                return "???";
            }}
            color: PublicTransport.warnAboutSection(modelData) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            elide: Text.ElideMiddle
        }

        // last row: arrival information
        RowLayout {
            visible: modelData.mode != JourneySection.Waiting
            QQC2.Label {
                text: Localizer.formatTime(modelData, "scheduledArrivalTime")
            }
            QQC2.Label {
                text: (modelData.arrivalDelay >= 0 ? "+" : "") + modelData.arrivalDelay
                color: modelData.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: modelData.hasExpectedArrivalTime
            }
        }
        RowLayout {
            visible: modelData.mode != JourneySection.Waiting
            QQC2.Label {
                text: modelData.to.name
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            QQC2.Label {
                text: modelData.hasExpectedArrivalPlatform ? modelData.expectedArrivalPlatform : modelData.scheduledArrivalPlatform
                color: modelData.arrivalPlatformChanged ? Kirigami.Theme.negativeTextColor
                    : modelData.hasExpectedArrivalPlatform ? Kirigami.Theme.positiveTextColor
                    : Kirigami.Theme.textColor
                visible: modelData.scheduledArrivalPlatform !== ""
            }
        }

        // optional bottom row: notes
        QQC2.Label {
            Layout.columnSpan: 2
            text: modelData.notes.join("<br/>")
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            visible: modelData.notes.length > 0
            font.italic: true
        }
    }
}
