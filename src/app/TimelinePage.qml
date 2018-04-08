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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    signal showBoardingPass(var pass, string passId)

    title: qsTr("My Itinerary")
    // context drawer content
    actions {
        contextualActions: [
            Kirigami.Action {
                text: qsTr("Delete Pass")
                iconName: "edit-delete"
                onTriggered: print("TODO")
            }
        ]
    }

    // page content
    Component {
        id: flightDelegate
        App.FlightDelegate {}
    }
    Component {
        id: hotelDelegate
        App.HotelDelegate {}
    }
    Component {
        id: trainDelegate
        App.TrainDelegate {}
    }
    Component {
        id: busDelegate
        App.BusDelegate {}
    }
    Component {
        id: todayDelegate
        Item {
            implicitHeight: visible ? label.implicitHeight : 0
            QQC2.Label {
                id: label
                anchors.fill: parent
                text: qsTr("Nothing on the itinerary for today.");
                color: Kirigami.Theme.textColor
                horizontalAlignment: Qt.AlignHCenter
            }
        }
    }

    ListView {
        model: _timelineModel
        spacing: 5

        delegate: Loader {
            property var modelData: model

            height: item.implicitHeight
            anchors.left: parent.left
            anchors.right: parent.right
            sourceComponent: {
                switch (modelData.type) {
                    case TimelineModel.Flight: return flightDelegate;
                    case TimelineModel.Hotel: return hotelDelegate;
                    case TimelineModel.TrainTrip: return trainDelegate;
                    case TimelineModel.BusTrip: return busDelegate;
                    case TimelineModel.TodayMarker: return todayDelegate;
                }
            }

            onLoaded: {
                if (modelData.type != TimelineModel.TodayMarker) {
                    item.reservation = Qt.binding(function() { return modelData.reservation; });
                    item.passId = Qt.binding(function() { return modelData.passId; });
                    item.pass = Qt.binding(function() { return modelData.pass; });
                    item.showBoardingPass.connect(onShowBoardingPass);
                } else {
                    item.visible = modelData.isTodayEmpty;
                }
            }
            function onShowBoardingPass(pass, passId) {
                root.showBoardingPass(pass, passId)
            }
        }

        section.property: "sectionHeader"
        section.delegate: Rectangle {
            color: Kirigami.Theme.backgroundColor
            implicitHeight: headerItem.implicitHeight
            implicitWidth: ListView.view.width
            Kirigami.BasicListItem {
                id: headerItem
                label: section
                icon: "view-calendar-day"
            }
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
    }
}
