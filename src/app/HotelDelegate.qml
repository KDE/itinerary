/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary
import org.kde.coreaddons as KCoreAddons


TimelineDelegate {
    id: root

    contentItem: ColumnLayout {
        id: summary

        readonly property var reservation: root.reservation
        readonly property bool isCheckout: root.rangeType === TimelineElement.RangeEnd
        readonly property var place: reservation?.reservationFor
        readonly property string checkin: formatStayDate("checkinTime")
        readonly property string checkout: formatStayDate("checkoutTime")
        readonly property string location: place ? [place.address.addressLocality,
            Localizer.formatCountryWithContext(place.address, null, "")].filter(part => part.length > 0).join(", ") : ""
        readonly property int nights: {
            const start = Util.dateTimeStripTimezone(summary.reservation, "checkinTime");
            const end = Util.dateTimeStripTimezone(summary.reservation, "checkoutTime");
            if (isNaN(start.getTime()) || isNaN(end.getTime()))
                return 0;
            // Compare calendar dates, so a daylight-saving transition is still one night.
            return Math.max(0, (Date.UTC(end.getFullYear(), end.getMonth(), end.getDate())
                - Date.UTC(start.getFullYear(), start.getMonth(), start.getDate())) / 86400000);
        }

        function formatStayDate(propertyName: string): string {
            const date = Util.dateTimeStripTimezone(summary.reservation, propertyName);
            if (isNaN(date.getTime()))
                return "";
            const day = Qt.formatDate(date, Qt.locale().dateFormat(Locale.LongFormat));
            return Util.isStartOfDay(summary.reservation, propertyName) ? day
                : i18nc("date and time of a hotel stay", "%1 · %2", day, KCoreAddons.Format.formatTime(summary.reservation, propertyName, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded));
        }

        enabled: !root.controller.isCanceled
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                source: ReservationHelper.defaultIconName(summary.reservation)
                Accessible.ignored: true
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    id: heading
                    Layout.fillWidth: true
                    level: 2
                    font.weight: Font.DemiBold
                    text: summary.isCheckout ? i18n("Check-out %1", summary.place?.name ?? "") : summary.place?.name ?? ""
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    visible: text.length > 0
                    text: summary.nights > 0
                        ? (summary.location.length > 0 ? i18nc("location and duration of a stay", "%1 · %2", summary.location, i18np("%1 night", "%1 nights", summary.nights))
                                                    : i18np("%1 night", "%1 nights", summary.nights))
                        : summary.location
                    textFormat: Text.PlainText
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                Layout.alignment: Qt.AlignTop
                visible: !summary.isCheckout && summary.checkin.length > 0
                text: i18nc("hotel checkin", "Check-in")
                opacity: 0.7
            }
            QQC2.Label {
                Layout.fillWidth: true
                visible: !summary.isCheckout && summary.checkin.length > 0
                text: summary.checkin
                wrapMode: Text.WordWrap
            }
            QQC2.Label {
                Layout.alignment: Qt.AlignTop
                visible: summary.checkout.length > 0
                text: i18nc("hotel checkout", "Check-out")
                opacity: 0.7
            }
            QQC2.Label {
                Layout.fillWidth: true
                visible: summary.checkout.length > 0
                text: summary.checkout
                wrapMode: Text.WordWrap
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            visible: text.length > 0
            text: summary.place ? Localizer.formatAddress(summary.place.address) : ""
            textFormat: Text.PlainText
            opacity: 0.7
            wrapMode: Text.WordWrap
        }
    }

    Accessible.name: heading.text
}
