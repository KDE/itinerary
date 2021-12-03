/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.i18n.localeData 1.0
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    property var tripGroup;
    property var tripGroupId;
    property var rangeType;

    property var weatherForecast: TripGroupInfoProvider.weatherForecast(tripGroup)

    id: root
    showClickFeedback: true
    topPadding: rangeType == TimelineElement.RangeEnd ? 0 : Kirigami.Units.largeSpacing

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
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
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Layout.preferredWidth
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

    contentItem: Column {
        id: contentLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18np("Date: %2 (one day)", "Date: %2 (%1 days)",
                       Math.ceil((root.tripGroup.endDateTime.getTime() - root.tripGroup.beginDateTime.getTime()) / (1000 * 3600 * 24)),
                       Localizer.formatDateTime(root.tripGroup, "beginDateTime"))
        }

        Row {
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
            model: TripGroupInfoProvider.locationInformation(tripGroup, Settings.homeCountryIsoCode)

            QQC2.Label {
                text: {
                    if (modelData.powerPlugCompatibility == LocationInformation.PartiallyCompatible) {
                        if (modelData.powerPlugTypes == "")
                            return i18n("%1: some incompatible power sockets (%2)", Country.fromAlpha2(modelData.isoCode).name, modelData.powerSocketTypes);
                        else
                            return i18n("%1: some incompatible power plugs (%2)", Country.fromAlpha2(modelData.isoCode).name, modelData.powerPlugTypes);
                    } else {
                        return i18n("%1: no compatible power plugs (%2)", Country.fromAlpha2(modelData.isoCode).name, modelData.powerSocketTypes);
                    }
                }
                color: modelData.powerPlugCompatibility == LocationInformation.PartiallyCompatible ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor
                wrapMode: Text.WordWrap
                width: contentLayout.width
            }
        }

        QQC2.Label {
            readonly property var currencies: TripGroupInfoProvider.currencies(tripGroup, Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode)
            text: currencies.length > 0 ? i18np("Currency: %2", "Currencies: %2", currencies.length, currencies.join(", ")) : ""
            visible: currencies.length > 0
        }

        Kirigami.Separator {
            visible: root.rangeType == TimelineElement.RangeBegin
        }

        Row {
            visible: root.rangeType == TimelineElement.RangeBegin
            anchors.right: parent.right
            QQC2.ToolButton {
                icon.name: "export-symbolic"
                onClicked: {
                    tripGroupGpxExportDialog.tripGroupId = root.tripGroupId
                    tripGroupGpxExportDialog.open()
                }
            }
            QQC2.ToolButton {
                icon.name: "edit-delete"
                onClicked: {
                    deleteTripGroupWarningSheet.tripGroupId = root.tripGroupId
                    deleteTripGroupWarningSheet.sheetOpen = true
                }
            }
        }
    }

    // hide content entirely in the header-only end elements
    Binding {
        target: contentItem.parent
        property: "visible"
        value: rangeType != TimelineElement.RangeEnd
    }

    onClicked: {
        if (rangeType == TimelineElement.SelfContained)
            TripGroupProxyModel.expand(tripGroupId);
        else
            TripGroupProxyModel.collapse(tripGroupId);
    }
}
