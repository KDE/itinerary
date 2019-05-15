/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    property var batchId;

    id: root
    title: i18n("Alternative Connections")

    Component.onCompleted: _journeyQueryModel.queryJourney(batchId);

    Component {
        id: sectionDelegate
        Kirigami.AbstractListItem {
            GridLayout {
                columns: 2

                // top row: departure time, departure location, departure platform
                RowLayout {
                    visible: modelData.mode != JourneySection.Waiting
                    QQC2.Label {
                        text: Localizer.formatTime(modelData, "scheduledDepartureTime")
                    }
                    QQC2.Label {
                        text: (modelData.departureDelay >= 0 ? "+" : "") + modelData.departureDelay
                        color: modelData.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                        visible: modelData.hasExpectedDepartureTime
                    }
                }
                RowLayout {
                    visible: modelData.mode != JourneySection.Waiting
                    QQC2.Label {
                        text: modelData.from.name
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        text: modelData.hasExpectedDeparturePlatform ? modelData.expectedDeparturePlatform : modelData.scheduledDeparturePlatform
                        color: modelData.departurePlatformChanged ? Kirigami.Theme.negativeTextColor
                            : modelData.hasExpectedDeparturePlatform ? Kirigami.Theme.positiveTextColor
                            : Kirigami.Theme.textColor
                        visible: modelData.scheduledDeparturePlatform !== ""
                    }
                }

                // middle row: mode symbol, transport mode, duration
                Rectangle {
                    color: modelData.route.line.hasColor ? modelData.route.line.color : "transparent"
                    implicitHeight: modeIcon.height
                    implicitWidth: modeIcon.width
                    Layout.alignment: Qt.AlignHCenter

                    Kirigami.Icon {
                        id: modeIcon
                        anchors.centerIn: parent
                        source: {
                            switch (modelData.mode) {
                                case JourneySection.PublicTransport:
                                    return PublicTransport.lineModeIcon(modelData.route.line.mode);
                                case JourneySection.Walking: return "qrc:///images/walk.svg";
                                case JourneySection.Waiting: return "qrc:///images/wait.svg";
                                case JourneySection.Transfer: return "qrc:///images/transfer.svg";
                                default: return "question";
                            }
                        }
                        color: modelData.route.line.hasTextColor ? modelData.route.line.textColor : Kirigami.Theme.textColor
                        width: Kirigami.Units.iconSizes.medium
                        height: width
                        isMask: true
                    }
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: {
                        switch (modelData.mode) {
                        case JourneySection.PublicTransport:
                            return modelData.route.line.modeString + " " + modelData.route.line.name + " (" + Localizer.formatDuration(modelData.duration) + ")";
                        case JourneySection.Walking:
                            return i18n("Walk (%1)", Localizer.formatDuration(modelData.duration))
                        case JourneySection.Transfer:
                            return i18n("Transfer (%1)", Localizer.formatDuration(modelData.duration))
                        case JourneySection.Waiting:
                            return i18n("Wait (%1)", Localizer.formatDuration(modelData.duration))
                        return "???";
                    }}
                }

                // last row: arrival information
                RowLayout {
                    visible: modelData.mode != JourneySection.Waiting
                    QQC2.Label {
                        text: Localizer.formatTime(modelData, "scheduledArrivalTime")
                    }
                    QQC2.Label {
                        text: (modelData.arrivalDelay >= 0 ? "+" : "") + modelData.arrivalDelay
                        color: modelData.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                        visible: modelData.hasExpectedArrivalTime
                    }
                }
                RowLayout {
                    visible: modelData.mode != JourneySection.Waiting
                    QQC2.Label {
                        text: modelData.to.name
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        text: modelData.hasExpectedArrivalPlatform ? modelData.expectedArrivalPlatform : modelData.scheduledArrivalPlatform
                        color: modelData.arrivalPlatformChanged ? Kirigami.Theme.negativeTextColor
                            : modelData.hasExpectedArrivalPlatform ? Kirigami.Theme.positiveTextColor
                            : Kirigami.Theme.textColor
                        visible: modelData.scheduledArrivalPlatform !== ""
                    }
                }
            }
        }
    }

    Component {
        id: journeyDelegate
        Kirigami.Card {
            id: top
            property var journey: model.journey

            header: Rectangle {
                id: headerBackground
                Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
                Kirigami.Theme.inherit: false
                color: Kirigami.Theme.backgroundColor
                implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
                anchors.leftMargin: -root.leftPadding
                anchors.topMargin: -root.topPadding
                anchors.rightMargin: -root.rightPadding

                RowLayout {
                    id: headerLayout
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.largeSpacing

                    QQC2.Label {
                        text: Localizer.formatTime(journey, "scheduledDepartureTime")
                        color: Kirigami.Theme.textColor
                        Layout.fillWidth: true
                    }

                    QQC2.Label {
                        text: Localizer.formatDuration(journey.duration)
                        color: Kirigami.Theme.textColor
                    }
                }
            }

            contentItem: ColumnLayout {
                ListView {
                    delegate: sectionDelegate
                    model: journeyView.currentIndex == index ? journey.sections : 0
                    implicitHeight: contentHeight
                    Layout.fillWidth: true
                    boundsBehavior: Flickable.StopAtBounds
                }
                QQC2.Label {
                    text: i18np("One change", "%1 changes", journey.numberOfChanges)
                    visible: journeyView.currentIndex != index
                }
            }

            // TODO add actions to update to this alternative
            onClicked: {
                journeyView.currentIndex = index;
            }

            actions: [
                Kirigami.Action {
                    text: i18n("Save")
                    iconName: "document-save"
                    onTriggered: {
                        _journeyQueryModel.saveJourney(batchId, index);
                        applicationWindow().pageStack.pop();
                    }
                    visible: journeyView.currentIndex == index
                }
            ]
        }
    }

    Kirigami.CardsListView {
        id: journeyView
        anchors.fill: parent
        clip: true
        delegate: journeyDelegate
        model: _journeyQueryModel

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: _journeyQueryModel.loading
        }

        QQC2.Label {
            anchors.centerIn: parent
            width: parent.width
            text: _journeyQueryModel.errorMessage
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }
    }
}
