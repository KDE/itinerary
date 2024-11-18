// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

RowLayout {
    id: root

    required property var departure
    required property var departureName
    required property string departureCountry
    property string departurePlatform
    property string transportName
    property string transportIcon
    property real progress
    property alias depTimeWidth: depTime.width
    required property var reservationFor

    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    Layout.fillWidth: true

    ColumnLayout {
        spacing: 0

        JourneySectionStopDelegateLineSegment {
            lineColor: root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
            isDeparture: true

            Layout.fillHeight: true
            leadingProgress: root.progress > 0 ? 1 : 0
            trailingProgress: root.progress > 0 ? 1 : 0
        }

        JourneySectionStopDelegateLineSegment {
            visible: departureCountryLayout.visible
            lineColor: root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
            hasStop: false
            leadingProgress: root.progress > 0 ? 1 : 0
            trailingProgress: root.progress > 0 ? 1 : 0

            Layout.fillHeight: true
        }
    }

    ColumnLayout {
        id: innerLayout

        spacing: 0

        Layout.bottomMargin: Kirigami.Units.largeSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.fillHeight: true
            Layout.fillWidth: true

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                Layout.alignment: Qt.AlignTop

                QQC2.Label {
                    id: depTime
                    text: Localizer.formatTime(reservationFor, "departureTime")
                }

                QQC2.Label {
                    text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
                    color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: departure.hasExpectedDepartureTime
                    Accessible.ignored: !visible
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: root.departureName
                    elide: Text.ElideRight
                }

                QQC2.Control {
                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        TransportIcon {
                            color: "white"
                            isMask: true
                            size: Kirigami.Units.iconSizes.smallMedium
                            source: root.transportIcon
                        }
                        QQC2.Label {
                            color: "white"
                            text: root.transportName
                            visible: root.transportName.length > 0
                        }
                    }
                    background: Rectangle {
                        radius: Kirigami.Units.cornerRadius
                        color: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    }
                }

                QQC2.Label {
                    text: departure.route.direction ? i18nc("Direction of the transport mode", "To %1", departure.route.direction) : ""
                    visible: departure.route.direction
                    Layout.fillWidth: true
                }
            }

            QQC2.Label {
                Layout.alignment: Qt.AlignRight | Qt.AlignTop

                text: root.departurePlatform
            }
        }

        RowLayout {
            id: departureCountryLayout

            spacing: Kirigami.Units.smallSpacing
            visible: departureCountryLabel.text.length > 0

            Item {
                Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
            }

            QQC2.Label {
                id: departureCountryLabel

                text: root.departureCountry
                Layout.fillWidth: true
            }
        }
    }
}
