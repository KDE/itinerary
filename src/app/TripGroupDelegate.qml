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

    property var weatherForecast: _tripGroupInfoProvider.weatherForecast(tripGroup)

    id: root
    showClickFeedback: true
    topPadding: rangeType == TimelineElement.RangeEnd ? 0 : Kirigami.Units.largeSpacing

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.rightMargin: -root.rightPadding
        anchors.bottomMargin: root.rangeType == TimelineElement.RangeEnd ? -root.bottomPadding : 0

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: switch (rangeType) {
                    case TimelineElement.SelfContained: return "go-next-symbolic";
                    case TimelineElement.RangeBegin: return "go-down-symbolic";
                    case TimelineElement.RangeEnd: return "go-up-symbolic";
                }
                width: Kirigami.Units.iconSizes.small
                height: width
                color: Kirigami.Theme.textColor
                isMask: true
            }
            QQC2.Label {
                text: root.rangeType == TimelineElement.RangeEnd ? i18n("End: %1", tripGroup.name) : i18n("Trip: %1", tripGroup.name)
                color: Kirigami.Theme.textColor
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }

    contentItem: ColumnLayout {
        id: contentLayout
        visible: root.rangeType != TimelineElement.RangeEnd

        QQC2.Label {
            text: i18np("Date: %2 (one day)", "Date: %2 (%1 days)",
                       Math.ceil((root.tripGroup.endDateTime.getTime() - root.tripGroup.beginDateTime.getTime()) / (1000 * 3600 * 24)),
                       Localizer.formatDateTime(root.tripGroup, "beginDateTime"))
        }

        RowLayout {
            visible: weatherForecast.valid

            Kirigami.Icon {
                source: weatherForecast.symbolIconName
                width: Kirigami.Units.iconSizes.small
                height: width
            }

            QQC2.Label {
                text: i18n("%1°C / %2°C", weatherForecast.minimumTemperature, weatherForecast.maximumTemperature)
            }
        }

        Repeater {
            model: _tripGroupInfoProvider.countryInformation(tripGroup, _settings.homeCountryIsoCode)

            QQC2.Label {
                text: {
                    if (modelData.powerPlugCompatibility == CountryInformation.PartiallyCompatible) {
                        if (modelData.powerPlugTypes == "")
                            return i18n("%1: some incompatible power sockets (%2)", Localizer.countryName(modelData.isoCode), modelData.powerSocketTypes);
                        else
                            return i18n("%1: some incompatible power plugs (%2)", Localizer.countryName(modelData.isoCode), modelData.powerPlugTypes);
                    } else {
                        return i18n("%1: no compatible power plugs (%2)", Localizer.countryName(modelData.isoCode), modelData.powerSocketTypes);
                    }
                }
                color: modelData.powerPlugCompatibility == CountryInformation.PartiallyCompatible ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor
                wrapMode: Text.WordWrap
            }
        }

        Component.onCompleted: {
            // hide content entirely in the header-only end elements
            parent.visible = contentLayout.visible
        }
    }

    onClicked: {
        if (rangeType == TimelineElement.SelfContained)
            _timelineModel.expand(tripGroupId);
        else
            _timelineModel.collapse(tripGroupId);
    }
}
