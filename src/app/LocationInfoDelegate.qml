/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.i18n.localeData
import org.kde.kitinerary as KItinerary
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormCard {
    id: root
    width: ListView.view.width

    property var locationInfo

    FormCard.AbstractFormDelegate {
        background: Rectangle {
            id: headerBackground
            color: Kirigami.Theme.neutralBackgroundColor
            Kirigami.Theme.colorSet: Kirigami.Theme.Header
            Kirigami.Theme.inherit: false
        }

        contentItem: ColumnLayout {
            id: headerLayout

            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.fillWidth: true

                // TODO appropriate header when we change timezone in the same country
                Kirigami.Icon {
                    source: root.locationInfo.dstDiffers ? "clock-symbolic" : "documentinfo-symbolic"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Layout.preferredWidth
                }

                Kirigami.Heading {
                    id: headerLabel
                    level: 3
                    text: root.locationInfo.dstDiffers ?
                        (root.locationInfo.isDst ? i18n("Daylight Saving Time") : i18nc("opposite to daylight saving time", "Standard Time"))
                        : i18n("Entering %1", Country.fromAlpha2(locationInfo.isoCode).name)
                    Layout.fillWidth: true
                    Accessible.ignored: true
                }
            }

            QQC2.Label {
                text: locationInfo.drivingSideLabel
                visible: locationInfo.drivingSideDiffers
                wrapMode: Text.WordWrap
                Accessible.ignored: !visible
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: visible ? i18n("No compatible power sockets: %1", locationInfo.powerSocketTypes) : ""
                color: Kirigami.Theme.negativeTextColor
                visible: locationInfo.powerPlugCompatibility == LocationInformation.Incompatible
                wrapMode: Text.WordWrap
                Accessible.ignored: !visible
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: visible ? i18n("Some incompatible power sockets: %1", locationInfo.powerSocketTypes) : ""
                visible: locationInfo.powerPlugCompatibility == LocationInformation.PartiallyCompatible && locationInfo.powerSocketTypes != ""
                wrapMode: Text.WordWrap
                Accessible.ignored: !visible
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: visible ? i18n("Some incompatible power plugs: %1", locationInfo.powerPlugTypes) : ""
                visible: locationInfo.powerPlugCompatibility == LocationInformation.PartiallyCompatible && locationInfo.powerPlugTypes != ""
                wrapMode: Text.WordWrap
                Accessible.ignored: !visible
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: visible ? i18n("Timezone change: %1 (%2)", locationInfo.timeZoneName,
                                     Localizer.formatTimeZoneOffset(locationInfo.timeZoneOffsetDelta)) : ""
                visible: locationInfo.timeZoneDiffers
                wrapMode: Text.WordWrap
                Accessible.ignored: !visible
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: {
                    if (!visible)
                        return "";
                    let rate = NaN;
                    let sourceValue = 1.0;
                    const homeCurrency = Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode;
                    if (Settings.performCurrencyConversion)
                        rate = UnitConversion.convertCurrency(sourceValue, homeCurrency, locationInfo.currencyCode);
                    while (rate < 1 && rate > 0) { // scale to a useful order of magnitude
                        rate *= 10;
                        sourceValue *= 10;
                    }
                    if (!isNaN(rate))
                        return i18nc("currency conversion rate", "Currency: %1 = %2",
                                     Localizer.formatCurrency(sourceValue, homeCurrency), Localizer.formatCurrency(rate, locationInfo.currencyCode));
                    return i18n("Currency: %1", locationInfo.currencyCode);
                }
                visible: locationInfo.currencyDiffers
                wrapMode: Text.WordWrap
                Accessible.ignored: !visible
                Layout.fillWidth: true
            }
        }
    }

    Accessible.name: headerLabel.text
}
