/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    property var stop
    property var dateTime

    title: i18n("Departures")
    actions: [
        Kirigami.Action {
            text: i18n("Earlier")
            icon.name: "go-up-symbolic"
            onTriggered: departureModel.queryPrevious()
            enabled: departureModel.canQueryPrevious
        },
        Kirigami.Action {
            text: i18n("Later")
            icon.name: "go-down-symbolic"
            onTriggered: departureModel.queryNext()
            enabled: departureModel.canQueryNext
        },

        Kirigami.Action { separator: true },

        Kirigami.Action {
            id: longDistanceModeAction
            text: i18nc("journey query search constraint, title", "Long distance trains")
            icon.source: PublicTransport.lineModeIcon(Line.LongDistanceTrain)
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: localTrainModeAction
            text: i18nc("journey query search constraint, title", "Local trains")
            icon.source: PublicTransport.lineModeIcon(Line.LocalTrain)
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: rapidTransitModeAction
            text: i18nc("journey query search constraint, title", "Rapid transit")
            icon.source: PublicTransport.lineModeIcon(Line.Tramway)
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: busModeAction
            text: i18nc("journey query search constraint, title", "Bus")
            icon.source: PublicTransport.lineModeIcon(Line.Bus)
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: ferryModeAction
            text: i18nc("journey query search constraint, title", "Ferry")
            icon.source: PublicTransport.lineModeIcon(Line.Ferry)
            checkable: true
            checked: true
        }
    ]

    function allLineModes()
    {
        for (const s of [longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction]) {
            if (!s.checked) {
                return false;
            }
        }
        return true;
    }

    function stopoverRequest()
    {
        let req = PublicTransport.stopoverRequestForPlace(stop, dateTime);
        let lineModes = [];
        if (!allLineModes()) {
            if (longDistanceModeAction.checked)
                lineModes.push(Line.LongDistanceTrain, Line.Train);
            if (localTrainModeAction.checked)
                lineModes.push(Line.LocalTrain);
            if (rapidTransitModeAction.checked)
                lineModes.push(Line.RapidTransit, Line.Metro, Line.Tramway, Line.RailShuttle);
            if (busModeAction.checked)
                lineModes.push(Line.Bus, Line.Coach);
            if (ferryModeAction.checked)
                lineModes.push(Line.Ferry, Line.Boat);
        }
        req.lineModes = lineModes;
        return req;
    }

    StopoverQueryModel {
        id: departureModel
        manager: LiveDataManager.publicTransportManager
        request: stopoverRequest();
    }

    Component {
        id: departureDelegate
        FormCard.FormCard {
            id: top
            required property var departure
            width: ListView.view.width
            enabled: top.departure.disruptionEffect !== Disruption.NoService

            FormCard.AbstractFormDelegate {
                contentItem: GridLayout {
                    id: contentLayout
                    columns: 2

                    // top row: departure time, departure location, departure platform
                    RowLayout {
                        QQC2.Label {
                            text: Localizer.formatTime(departure, "scheduledDepartureTime")
                        }
                        QQC2.Label {
                            text: {
                                if (top.departure.disruptionEffect == Disruption.NoService)
                                    return i18nc("a train/bus journey canceled by its operator", "Canceled");
                                return (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay;
                            }
                            color: {
                                if (top.departure.departureDelay > 1 || top.departure.disruptionEffect == Disruption.NoService)
                                    return Kirigami.Theme.negativeTextColor;
                                return Kirigami.Theme.positiveTextColor;
                            }
                            // Keeping it visible so the layout is more uniform
                            opacity: (top.departure.hasExpectedDepartureTime || top.departure.disruptionEffect == Disruption.NoService) ? 1 : 0
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
    }


    ListView {
        id: journeyView
        anchors.fill: parent
        clip: true
        delegate: departureDelegate
        model: departureModel
        spacing: Kirigami.Units.largeSpacing

        header: VerticalNavigationButton {
            visible: departureModel.canQueryPrevious
            width: journeyView.width
            text: i18nc("@action:button", "Load earlier departures")
            iconName: "go-up-symbolic"
            onClicked: departureModel.queryPrevious()
        }

        footer: VerticalNavigationButton {
            visible: departureModel.canQueryNext
            width: journeyView.width
            iconName: "go-down-symbolic"
            text: i18nc("@action:button", "Load later connections")
            onClicked: departureModel.queryNext()

            FormCard.FormCard {
                visible: departureModel.attributions.length > 0

                Layout.fillWidth: true

                FormCard.FormTextDelegate {
                    text: i18n("Data providers:")
                    description: PublicTransport.attributionSummary(departureModel.attributions)
                    onLinkActivated: Qt.openUrlExternally(link)
                }
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
