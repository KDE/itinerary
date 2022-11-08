/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.i18n.localeData 1.0
import org.kde.kitinerary 1.0 as KItinerary
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    id: root
    property alias locationInfo: _controller.locationInformation
    property QtObject controller: LocationInformationDelegateController {
        id: _controller
        homeCurrencyCode: Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode
        performCurrencyConversion: Settings.performCurrencyConversion
    }

   header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.neutralBackgroundColor
        radius: Kirigami.Units.smallSpacing
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
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Layout.preferredWidth
                isMask: true
            }
            QQC2.Label {
                text: i18n("Entering %1", Country.fromAlpha2(locationInfo.isoCode).name)
                Layout.fillWidth: true
            }
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            width: topLayout.width
            text: locationInfo.drivingSideLabel
            visible: locationInfo.drivingSideDiffers
            wrapMode: Text.WordWrap
        }

        QQC2.Label {
            width: topLayout.width
            text: visible ? i18n("No compatible power sockets: %1", locationInfo.powerSocketTypes) : ""
            color: Kirigami.Theme.negativeTextColor
            visible: locationInfo.powerPlugCompatibility == LocationInformation.Incompatible
            wrapMode: Text.WordWrap
        }
        QQC2.Label {
            width: topLayout.width
            text: visible ? i18n("Some incompatible power sockets: %1", locationInfo.powerSocketTypes) : ""
            visible: locationInfo.powerPlugCompatibility == LocationInformation.PartiallyCompatible && locationInfo.powerSocketTypes != ""
            wrapMode: Text.WordWrap
        }
        QQC2.Label {
            width: topLayout.width
            text: visible ? i18n("Some incompatible power plugs: %1", locationInfo.powerPlugTypes) : ""
            visible: locationInfo.powerPlugCompatibility == LocationInformation.PartiallyCompatible && locationInfo.powerPlugTypes != ""
            wrapMode: Text.WordWrap
        }

        QQC2.Label {
            width: topLayout.width
            text: visible ? i18n("Timezone change: %1 (%2)", locationInfo.timeZoneName, Localizer.formatDuration(locationInfo.timeZoneOffsetDelta)) : ""
            visible: locationInfo.timeZoneDiffers
            wrapMode: Text.WordWrap
        }

        QQC2.Label {
            width: topLayout.width
            text: visible ? i18n("Currency: %1", controller.hasCurrencyConversion ? controller.currencyConversionLabel : locationInfo.currencyCode) : ""
            visible: locationInfo.currencyDiffers
            wrapMode: Text.WordWrap
        }

    }
}

