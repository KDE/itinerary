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
    title: "Departure Query"
    reachableModeEnabled: false

    width: 540
    height: 800

    pageStack.initialPage: departureQueryPage

    ListModel {
        id: exampleModel
        ListElement { name: "Paris Charles de Gaulle"; lon: 2.57110; lat: 49.00406 }
        ListElement { name: "Paris Gare de Lyon"; lon: 2.37385; lat: 48.84467 }
        ListElement { name: "Zürich Flughafen"; lon: 8.56275; lat: 47.45050 }
        ListElement { name: "Randa"; lon: 7.78315; lat:  46.09901 }
        ListElement { name: "Brussels Gare de Midi"; lon: 4.33620; lat: 50.83588 }
        ListElement { name: "ULB"; lon: 4.38116; lat: 50.81360 }
        ListElement { name: "Wien Flughafen"; lon: 16.56312; lat: 48.12083 }
        ListElement { name: "Wien Hauptbahnhof"; lon: 16.37859; lat: 48.18282 }
        ListElement { name: "Akademy 2018 BBQ"; lon: 16.43191; lat: 48.21612 }
        ListElement { name: "LEI"; lon: -2.37251; lat: 36.84774; }
        ListElement { name: "Akademy 2017 Accomodation"; lon: -2.44788; lat: 36.83731 }
        ListElement { name: "Akademy 2017 Venue"; lon: -2.40377; lat: 36.82784 }
        ListElement { name: "Berlin Flughafen Tegel"; lon: 13.29281; lat: 52.55420; }
        ListElement { name: "Berlin Alexanderplatz"; lon: 13.41644; lat: 52.52068 }
        ListElement { name: "Berlin Flughafen Schönefeld"; lon: 13.51870; lat: 52.38841 }
        ListElement { name: "Brno central station"; lon: 16.61287; lat: 49.19069 }
        ListElement { name: "Akademy 2014 venue"; lon: 16.57564; lat: 49.22462 }
        ListElement { name: "Copenhagen Central"; lon: 12.56489; lat: 55.67238 }
        ListElement { name: "Frankfurt (Main) Hauptbahnhof"; lon: 8.6625; lat: 50.106944 }
        ListElement { name: "Schiphol Airport"; lon: 4.7611; lat: 52.309 }
    }

    Component {
        id: departureDelegate
        Kirigami.AbstractListItem {
            RowLayout {
                id: delegateLayout

                Rectangle {
                    id: colorBar
                    width: Kirigami.Units.largeSpacing
                    color: modelData.route.line.hasColor ? modelData.route.line.color : "transparent"
                    Layout.fillHeight: true
                }

                QQC2.Label {
                    text: {
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
                    }
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 2
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: modelData.route.line.modeString + " " + modelData.route.line.name + " to " + modelData.route.direction
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
                        QQC2.Label {
                            text: "Departure: " + modelData.scheduledDepartureTime.toTimeString()
                        }
                        QQC2.Label {
                            text: (modelData.departureDelay >= 0 ? "+" : "") + modelData.departureDelay
                            color: modelData.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: modelData.hasExpectedDepartureTime
                        }
                    }
                    RowLayout {
                        QQC2.Label {
                            text: "From: " + modelData.stopPoint.name
                        }
                        QQC2.Label {
                            visible: modelData.scheduledPlatform != ""
                            text: "Platform: " + modelData.scheduledPlatform + (platformChange.visible ? " -> " : "")
                            color: (!platformChange.visible && modelData.hasExpectedPlatform) ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
                        }
                        QQC2.Label {
                            id: platformChange
                            text: modelData.expectedPlatform
                            visible: modelData.hasExpectedPlatform && modelData.scheduledPlatform != modelData.expectedPlatform
                            color: Kirigami.Theme.negativeTextColor
                        }
                    }
                }
            }
        }
    }

    Component {
        id: departureQueryPage
        Kirigami.Page {
            ColumnLayout {
                anchors.fill: parent

                QQC2.CheckBox {
                    id: arrivalBox
                    text: checked ? "Arrival" : "Departure"
                }

                QQC2.CheckBox {
                    text: "Allow insecure backends"
                    onToggled: _queryMgr.setAllowInsecure(checked)
                }

                QQC2.ComboBox {
                    id: exampleSelector
                    Layout.fillWidth: true
                    model: exampleModel
                    textRole: "name"
                    onCurrentIndexChanged: {
                        var obj = exampleModel.get(currentIndex);
                        nameQuery.text = obj.name
                        lonQuery.text = obj.lon
                        latQuery.text = obj.lat
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    QQC2.TextField {
                        Layout.fillWidth: true
                        id: nameQuery
                    }
                    QQC2.Button {
                        text: "Query"
                        onClicked: _queryMgr.queryDeparture(nameQuery.text, NaN, NaN, arrivalBox.checked);
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    QQC2.TextField {
                        id: lonQuery
                    }
                    QQC2.TextField {
                        id: latQuery
                    }
                    QQC2.Button {
                        text: "Query"
                        onClicked: _queryMgr.queryDeparture("", latQuery.text, lonQuery.text, arrivalBox.checked);

                    }
                }

                ListView {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    model: _departures
                    clip: true
                    delegate: departureDelegate

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
