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

    Component {
        id: journyQueryPage
        Kirigami.Page {
            ColumnLayout {
                anchors.fill: parent
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
                                        return modelData.route.line.modeString + " " + modelData.route.line.name;
                                    case JourneySection.Walking:
                                        return "Walk";
                                    case JourneySection.Transfer:
                                        return "Transfer";
                                    case JourneySection.Waiting:
                                        return "Wait";
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
