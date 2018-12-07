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
            fromLat: 2.57110
            fromLon: 49.00406
            toLat: 2.37708
            toLon: 48.84388
        }
        ListElement {
            name: "ZRH -> Randa"
            fromLat: 8.56275
            fromLon: 47.45050
            toLat: 7.78315
            toLon:  46.09901
        }
        ListElement {
            name: "Gare de Midi -> Fosdem"
            fromLat: 4.33620
            fromLon: 50.83588
            toLat: 4.38116
            toLon: 50.81360
        }
        ListElement {
            name: "VIE -> Akademy 2018 Accomodation"
            fromLat: 16.56312
            fromLon: 48.12083
            toLat: 16.37859
            toLon: 48.18282
        }
        ListElement {
            name: "Akademy 2018 BBQ -> Accomodation"
            fromLat: 16.43191
            fromLon: 48.21612
            toLat: 16.37859
            toLon: 48.18282
        }
        ListElement {
            name: "LEI -> Akademy 2017 Accomodation"
            fromLat: -2.37251
            fromLon: 36.84774
            toLat: -2.44788
            toLon: 36.83731
        }
        ListElement {
            name: "Akademy 2017 Venue -> Accomodation"
            fromLat: -2.40377
            fromLon: 36.82784
            toLat: -2.44788
            toLon: 36.83731
        }
        ListElement {
            name: "TXL -> Akademy 2016"
            fromLat: 13.29281
            fromLon: 52.55420
            toLat: 13.41644
            toLon: 52.52068
        }
        ListElement {
            name: "SXF -> Akademy 2016"
            fromLat: 13.51870
            fromLon: 52.38841
            toLat: 13.41644
            toLon: 52.52068
        }
        ListElement {
            name: "Brno central station -> Akademy 2014 venue"
            fromLat: 16.61287
            fromLon: 49.19069
            toLat: 16.57564
            toLon: 49.22462
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
                    model: _journeys
                }

                ListView {
                    Layout.fillHeight: true
                    model: _journeys[journeySelector.currentIndex].sections
                    delegate: Item {
                        implicitHeight: delegateLayout.implicitHeight
                        implicitWidth: delegateLayout.implicitWidth
                        ColumnLayout {
                            id: delegateLayout
                            Text {
                                text: "From: " + modelData.from.name
                                visible: index == 0
                            }
                            Text {
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
                            Text {
                                text: "To: " + modelData.to.name
                            }
                        }
                        Rectangle {
                            anchors.left: parent.left
                            anchors.leftMargin: -8
                            height: parent.height
                            width: 4
                            color: modelData.route.line.color
                        }
                    }
                }

            }
        }
    }
}
