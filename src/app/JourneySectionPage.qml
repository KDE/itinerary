/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kpublictransport
import org.kde.itinerary


Kirigami.Page {
    id: root
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    title: i18n("Journey Details")

    property var journeySection
    property alias showProgress: sectionModel.showProgress
    default property alias _children: root.children

    Kirigami.ColumnView.preventStealing: view.currentItem.objectName === "sectionMap"

    padding: 0
    header: ColumnLayout {
        spacing: 0
        visible: root.journeySection != undefined

        GridLayout {
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rows: 4
            rowSpacing: 0

            Layout.margins: Kirigami.Units.gridUnit

            TransportIcon {
                Layout.rowSpan: 4
                Layout.alignment: Qt.AlignTop
                Layout.rightMargin: Kirigami.Units.largeSpacing
                id: icon
                source: PublicTransport.lineIcon(journeySection.route.line)
                Layout.preferredWidth: height
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                isMask: !journeySection.route.line.hasLogo && !journeySection.route.line.hasModeLogo
            }

            QQC2.Label {
                Layout.row: 0
                Layout.column: 1
                Layout.fillWidth: true
                text: "<b>" + journeySection.route.line.modeString + " " + journeySection.route.line.name + "</b>"
            }

            QQC2.Label {
                Layout.row: 1
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Direction: %1", journeySection.route.direction)
                visible: journeySection.route.direction !== ""
            }

            QQC2.Label {
                Layout.row: 2
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Distance: %1", Localizer.formatDistance(journeySection.distance))
                visible: journeySection.distance > 0
            }
            QQC2.Label {
                Layout.row: 3
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Average Speed: %1", Localizer.formatSpeed(journeySection.distance / journeySection.duration * 3.6))
                visible: journeySection.distance > 0 && journeySection.duration > 0
            }
            QQC2.Label {
                Layout.row: 4
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("CO₂: %1", Localizer.formatWeight(journeySection.co2Emission))
                visible: journeySection.co2Emission > 0
            }

            QQC2.Label {
                id: notesLabel
                Layout.row: 5
                Layout.column: 1
                Layout.columnSpan: 2
                Layout.fillWidth: true
                text: journeySection.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignTop
                // Doesn't work with RichText.
                elide: Text.ElideRight
                maximumLineCount: 10
                Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount
                clip: true
                visible: journeySection.notes.length > 0
                font.italic: true
                onLinkActivated: Qt.openUrlExternally(link)

                Kirigami.OverlaySheet {
                    id: moreNotesSheet
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: journeySection.notes.join("<br/>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                }
            }

            Kirigami.LinkButton {
                Layout.row: 6
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18nc("@action:button", "Show More…")
                visible: notesLabel.implicitHeight > notesLabel.height
                onClicked: {
                    moreNotesSheet.open();
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }
    }
    JourneySectionModel {
        id: sectionModel
        journeySection: root.journeySection

    }
    ColumnLayout{
        anchors.fill: parent
        spacing: 0

        QQC2.SwipeView {
            id: view
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            Item {
                id: listPage

                objectName: "sectionList"

                QQC2.ScrollView{
                    id: scrollview
                    anchors.fill: parent

                    ListView {
                        clip: true
                        model: sectionModel
                        header: JourneySectionStopDelegate {
                            stop: journeySection.departure
                            isDeparture: true
                            trailingProgress: sectionModel.departureTrailingProgress
                            stopoverPassed: sectionModel.departed
                            Binding {
                                target: sectionModel
                                property: "departureTrailingSegmentLength"
                                value: trailingSegmentLength
                            }
                            visible: root.journeySection != undefined
                        }
                        delegate: JourneySectionStopDelegate {
                            stop: model.stopover
                            leadingProgress: model.leadingProgress
                            trailingProgress: model.trailingProgress
                            stopoverPassed: model.stopoverPassed
                            Binding {
                                target: model
                                property: "leadingLength"
                                value: leadingSegmentLength
                            }
                            Binding {
                                target: model
                                property: "trailingLength"
                                value: trailingSegmentLength
                            }
                        }
                        footer: JourneySectionStopDelegate {
                            stop: journeySection.arrival
                            isArrival: true
                            leadingProgress: sectionModel.arrivalLeadingProgress
                            stopoverPassed: sectionModel.arrived
                            Binding {
                                target: sectionModel
                                property: "arrivalLeadingSegmentLength"
                                value: leadingSegmentLength
                            }
                            visible: root.journeySection != undefined
                        }
                    }
                }
            }
            Item {
                id: mapPage

                objectName: "sectionMap"

                MapView {
                    id: map
                    anchors.fill: parent
                    QtLocation.MapPolyline {
                        id: line
                        line.width: 10
                        line.color: journeySection.route.line.hasColor ? journeySection.route.line.color : Kirigami.Theme.textColor
                        path: PublicTransport.pathToGeoCoordinates(journeySection)
                        }

                    QtLocation.MapQuickItem {
                        coordinate {

                           latitude: journeySection.departure.stopPoint.latitude
                           longitude: journeySection.departure.stopPoint.longitude
                        }
                        anchorPoint.x: sourceItem.width/2
                        anchorPoint.y: sourceItem.height/2
                        sourceItem: Rectangle {
                            width:15
                            height:15
                            radius: height/2
                            border.width: 2
                            border.color: line.line.color
                            }
                        }

                    Repeater {
                        model: sectionModel

                        QtLocation.MapQuickItem {
                            coordinate {

                               latitude: model.stopover.stopPoint.latitude
                               longitude: model.stopover.stopPoint.longitude
                            }
                            anchorPoint.x: sourceItem.width/2
                            anchorPoint.y: sourceItem.height/2
                            sourceItem: Rectangle {
                                width: 6
                                height: 6
                                radius: height/2
                                opacity: 0.5
                            }
                        }
                    }
                    QtLocation.MapQuickItem {
                        coordinate {

                           latitude: journeySection.arrival.stopPoint.latitude
                           longitude: journeySection.arrival.stopPoint.longitude
                        }
                        anchorPoint.x: sourceItem.width/2
                        anchorPoint.y: sourceItem.height/2

                        Component.onCompleted: {
                            map.center.latitude = line.path[(line.pathLength()/2).toFixed(0)].latitude
                            map.center.longitude = line.path[(line.pathLength()/2).toFixed(0)].longitude
                        }
                        sourceItem: Rectangle {
                            width:15
                            height:15
                            radius: height/2
                            border.width: 2
                            border.color: line.line.color
                        }
                    }
                }
            }
        }
    }

    Components.FloatingButton {
        icon.name: checked ? "format-list-unordered" : "map-gnomonic"
        text: i18nc("@action:button", "Show Map")
        onClicked: view.currentIndex === 0 ? view.currentIndex = 1 : view.currentIndex = 0
        checkable: true
        checked: view.currentIndex === 1

        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (scrollview.QQC2.ScrollBar && scrollview.QQC2.ScrollBar.vertical ? scrollview.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }

    }


}
