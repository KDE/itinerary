/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    property var stop
    property var dateTime

    id: root
    title: i18n("Departures")
    contextualActions: [
        Kirigami.Action {
            text: i18n("Earlier")
            iconName: "go-up-symbolic"
            onTriggered: departureModel.queryPrevious()
            enabled: departureModel.canQueryPrevious
        },
        Kirigami.Action {
            text: i18n("Later")
            iconName: "go-down-symbolic"
            onTriggered: departureModel.queryNext()
            enabled: departureModel.canQueryNext
        }
    ]

    StopoverQueryModel {
        id: departureModel
        manager: LiveDataManager.publicTransportManager
        request: PublicTransport.stopoverRequestForPlace(stop, dateTime)
    }

    Component {
        id: departureDelegate
        Kirigami.AbstractListItem {
            highlighted: false
            GridLayout {
                columns: 2

                // top row: departure time, departure location, departure platform
                RowLayout {
                    QQC2.Label {
                        text: Localizer.formatTime(departure, "scheduledDepartureTime")
                    }
                    QQC2.Label {
                        text: {
                            if (departure.disruption == Disruption.NoService)
                                return i18nc("a train/bus journey canceled by its operator", "Canceled");
                            return (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay;
                        }
                        color: {
                            if (departure.departureDelay > 1 || departure.disruption == Disruption.NoService)
                                return Kirigami.Theme.negativeTextColor;
                            return Kirigami.Theme.positiveTextColor;
                        }
                        visible: departure.hasExpectedDepartureTime || departure.disruption == Disruption.NoService
                    }
                }
                RowLayout {
                    QQC2.Label {
                        text: departure.stopPoint.name
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        text: departure.hasExpectedPlatform ? departure.expectedPlatform : departure.scheduledPlatform
                        color: departure.departurePlatformChanged ? Kirigami.Theme.negativeTextColor
                            : departure.hasExpectedPlatform ? Kirigami.Theme.positiveTextColor
                            : Kirigami.Theme.textColor
                        visible: departure.scheduledPlatform !== ""
                    }
                }

                // middle row: mode symbol, transport mode, duration
                Rectangle {
                    color: (departure.route.line.hasColor && modeIcon.isMask) ? departure.route.line.color : "transparent"
                    implicitHeight: Kirigami.Units.iconSizes.smallMedium
                    implicitWidth: modeIcon.width
                    Layout.alignment: Qt.AlignHCenter

                    Kirigami.Icon {
                        id: modeIcon
                        anchors.centerIn: parent
                        source: PublicTransport.lineIcon(departure.route.line);
                        color: departure.route.line.hasTextColor ? departure.route.line.textColor : Kirigami.Theme.textColor
                        width: (departure.route.line.hasLogo || departure.route.line.hasModeLogo) ? implicitWidth : height
                        height: parent.height
                        isMask: !departure.route.line.hasLogo && !departure.route.line.hasModeLogo
                    }
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: departure.route.line.modeString + " " + departure.route.line.name;
                }

                // last row: arrival information
                RowLayout {
                    QQC2.Label {
                        text: i18nc("destination", "To:")
                    }
                }
                RowLayout {
                    QQC2.Label {
                        text: departure.route.direction
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }

                // optional bottom row: notes if present
                QQC2.Label {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: departure.notes.join("<br/>")
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    visible: departure.notes.length > 0
                    font.italic: true
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }
    }


    ListView {
        id: journeyView
        anchors.fill: parent
        clip: true
        delegate: departureDelegate
        model: departureModel

        header: QQC2.ToolButton {
            icon.name: "go-up-symbolic"
            visible: departureModel.canQueryPrevious
            onClicked: departureModel.queryPrevious()
            x: Kirigami.Units.largeSpacing * 2
            width: journeyView.width - Kirigami.Units.largeSpacing * 4
        }

        footer: ColumnLayout {
            x: Kirigami.Units.largeSpacing * 2
            width: journeyView.width - Kirigami.Units.largeSpacing * 4
            QQC2.ToolButton {
                Layout.fillWidth: true
                icon.name: "go-down-symbolic"
                visible: departureModel.canQueryNext
                onClicked: departureModel.queryNext()
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("Data providers: %1", PublicTransport.attributionSummary(departureModel.attributions))
                visible: departureModel.attributions.length > 0
                wrapMode: Text.Wrap
                font.pointSize: Kirigami.Units.pointSize * 0.8
                font.italic: true
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: departureModel.loading
        }

        QQC2.Label {
            anchors.centerIn: parent
            width: parent.width
            text: departureModel.errorMessage
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }
    }
}
