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
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kpublictransport 1.0

Kirigami.ApplicationWindow {
    title: "Journey Query"
    reachableModeEnabled: false

    width: 480
    height: 720

    pageStack.initialPage: journyQueryPage

    ListModel {
        id: exampleModel
        ListElement {
            name: "CDG -> Gare de Lyon"
            fromLon: 2.57110
            fromLat: 49.00406
            toLon: 2.37708
            toLat: 48.84388
        }
        ListElement {
            name: "ZRH -> Randa"
            fromLon: 8.56275
            fromLat: 47.45050
            toLon: 7.78315
            toLat:  46.09901
        }
        ListElement {
            name: "Gare de Midi -> Fosdem"
            fromLon: 4.33620
            fromLat: 50.83588
            toLon: 4.38116
            toLat: 50.81360
        }
        ListElement {
            name: "VIE -> Akademy 2018 Accomodation"
            fromLon: 16.56312
            fromLat: 48.12083
            toLon: 16.37859
            toLat: 48.18282
        }
        ListElement {
            name: "Akademy 2018 BBQ -> Accomodation"
            fromLon: 16.43191
            fromLat: 48.21612
            toLon: 16.37859
            toLat: 48.18282
        }
        ListElement {
            name: "LEI -> Akademy 2017 Accomodation"
            fromLon: -2.37251
            fromLat: 36.84774
            toLon: -2.44788
            toLat: 36.83731
        }
        ListElement {
            name: "Akademy 2017 Venue -> Accomodation"
            fromLon: -2.40377
            fromLat: 36.82784
            toLon: -2.44788
            toLat: 36.83731
        }
        ListElement {
            name: "TXL -> Akademy 2016"
            fromLon: 13.29281
            fromLat: 52.55420
            toLon: 13.41644
            toLat: 52.52068
        }
        ListElement {
            name: "SXF -> Akademy 2016"
            fromLon: 13.51870
            fromLat: 52.38841
            toLon: 13.41644
            toLat: 52.52068
        }
        ListElement {
            name: "Brno central station -> Akademy 2014 venue"
            fromLon: 16.61287
            fromLat: 49.19069
            toLon: 16.57564
            toLat: 49.22462
        }
    }

    function displayDuration(dur)
    {
        if (dur < 60)
            return "<1min";
        if (dur < 3600)
            return Math.floor(dur/60) + "min";
        return Math.floor(dur/3600) + ":" + Math.floor((dur % 3600)/60)
    }

    Component {
        id: journeyDelegate
        Item {
            implicitHeight: topLayout.implicitHeight
            implicitWidth: topLayout.implicitWidth

            RowLayout {
                id: topLayout

                Rectangle {
                    id: colorBar
                    width: Kirigami.Units.largeSpacing
                    color: modelData.route.line.color
                    Layout.fillHeight: true
                }

                QQC2.Label {
                    text: {
                        switch (modelData.mode) {
                            case JourneySection.PublicTransport:
                            {
                                switch (modelData.route.line.mode) {
                                    case Line.Air: return "✈️";
                                    case Line.Boat: return "🛥️";
                                    case Line.Bus: return "🚍";
                                    case Line.BusRapidTransit: return "🚌";
                                    case Line.Coach: return "🚌";
                                    case Line.Ferry: return "⛴️";
                                    case Line.Funicular: return "🚞";
                                    case Line.LocalTrain: return "🚆";
                                    case Line.LongDistanceTrain: return "🚄";
                                    case Line.Metro: return "🚇";
                                    case Line.RailShuttle: return "🚅";
                                    case Line.RapidTransit: return "🚊";
                                    case Line.Shuttle: return "🚐";
                                    case Line.Taxi: return "🚕";
                                    case Line.Train: return "🚆";
                                    case Line.Tramway: return "🚈";
                                    default: return "?";
                                }
                                break;
                            }
                            case JourneySection.Walking: return "🚶";
                            case JourneySection.Waiting: return "⌛";
                            case JourneySection.Transfer: return "⇄";
                            default: return "?";
                        }
                    }
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 2
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    RowLayout {
                        QQC2.Label {
                            text: "From: " + modelData.from.name + " Platform: " + modelData.scheduledDeparturePlatform
                        }
                        QQC2.Label {
                            text: modelData.expectedDeparturePlatform
                            color: modelData.departurePlatformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: modelData.hasExpectedDeparturePlatform
                        }
                    }
                    RowLayout {
                        QQC2.Label {
                            text: "Departure: " + modelData.scheduledDepartureTime.toTimeString()
                        }
                        QQC2.Label {
                            text: (modelData.departureDelay >= 0 ? "+" : "") + modelData.departureDelay
                            color: modelData.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: modelData.hasExpectedDepartureTime
                        }
                    }
                    QQC2.Label {
                        text: {
                            switch (modelData.mode) {
                            case JourneySection.PublicTransport:
                                return modelData.route.line.modeString + " " + modelData.route.line.name + " " + displayDuration(modelData.duration);
                            case JourneySection.Walking:
                                return "Walk " + displayDuration(modelData.duration)
                            case JourneySection.Transfer:
                                return "Transfer " + displayDuration(modelData.duration)
                            case JourneySection.Waiting:
                                return "Wait " + displayDuration(modelData.duration)
                            return "???";
                        }}
                    }
                    RowLayout {
                        QQC2.Label {
                            text: "To: " + modelData.to.name + " Platform: " + modelData.scheduledArrivalPlatform
                        }
                        QQC2.Label {
                            text: modelData.expectedArrivalPlatform
                            color: modelData.arrivalPlatformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: modelData.hasExpectedArrivalPlatform
                        }
                    }
                    RowLayout {
                        QQC2.Label {
                            text: "Arrival: " + modelData.scheduledArrivalTime.toTimeString()
                        }
                        QQC2.Label {
                            text: (modelData.arrivalDelay >= 0 ? "+" : "") + modelData.arrivalDelay
                            color: modelData.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: modelData.hasExpectedArrivalTime
                        }
                    }
                }
            }
        }
    }

    Component {
        id: journyQueryPage
        Kirigami.Page {
            ColumnLayout {
                anchors.fill: parent
                QQC2.ComboBox {
                    id: exampleSelector
                    Layout.fillWidth: true
                    model: exampleModel
                    textRole: "name"
                    onCurrentIndexChanged: {
                        var obj = exampleModel.get(currentIndex);
                        _queryMgr.findJourney(obj.fromLat, obj.fromLon, obj.toLat, obj.toLon);
                    }
                }
                QQC2.ComboBox {
                    id: journeySelector
                    Layout.fillWidth: true
                    model: _journeys
                }

                ListView {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    model: _journeys[journeySelector.currentIndex].sections
                    spacing: Kirigami.Units.smallSpacing
                    clip: true
                    delegate: journeyDelegate

                    QQC2.BusyIndicator {
                        anchors.centerIn: parent
                        running: _queryMgr.loading
                    }

                    QQC2.Label {
                        anchors.centerIn: parent
                        width: parent.width
                        text: _queryMgr.errorMessage
                        color: Kirigami.Theme.negativeTextColor
                        wrapMode: Text.Wrap
                    }
                }

            }
        }
    }
}
