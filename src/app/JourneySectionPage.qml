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
import org.kde.kirigamiaddons.formcard as FormCard
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
    SheetDrawer {
        id: sheetDrawer
        property var stop
        property bool isDeparture: false
        property bool isArrival: false
        anchors.fill: parent
        headerItem: Component {
            RowLayout {
                id: headerLayout
                Layout.preferredWidth: Kirigami.Units.gridUnit * 25

                Kirigami.Heading {
                    text: sheetDrawer.stop.stopPoint.name
                    Layout.fillWidth: true
                    elide: Qt.ElideRight
                }
                QQC2.Label {
                    id: departureTime
                    Layout.rightMargin: delayLabel.visible ? 0:Kirigami.Units.largeSpacing
                    text: Localizer.formatTime(sheetDrawer.stop, "scheduledDepartureTime")
//                    visible: sheetDrawer.stop.scheduledDepartureTime > 0
                    font.strikeout: sheetDrawer.stop.disruptionEffect === Disruption.NoService
                }

                QQC2.Label {
                    id: delayLabel
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    text: (sheetDrawer.stop.departureDelay >= 0 ? "+" : "") + sheetDrawer.stop.departureDelay
                    color: sheetDrawer.stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: departureTime.visible && sheetDrawer.stop.hasExpectedDepartureTime && sheetDrawer.stop.disruptionEffect !== Disruption.NoService
                }
            }
        }



        contentItem: Component{

            ColumnLayout {
                id: contentLayout
                width: parent.width
                FormCard.FormTextDelegate {
                    Layout.fillWidth: true
                    id: platformDelegate
                    text: i18n("Platform:")
                    description: {
                        const platform = sheetDrawer.stop.hasExpectedPlatform ? sheetDrawer.stop.expectedPlatform : sheetDrawer.stop.scheduledPlatform;

                        if (platform.length === 0) {
                            return '';
                        }

                        switch (sheetDrawer.stop.route.line.mode) {
                            case Line.Train:
                            case Line.Funicular:
                            case Line.LocalTrain:
                            case Line.LongDistanceTrain:
                            case Line.Metro:
                            case Line.RailShuttle:
                            case Line.RapidTransit:
                            case Line.Tramway:
                                return platform
                            case Line.Ferry:
                                return platform
                            default:
                                return platform
                        }
                    }
                    visible: description
                }
                FormCard.AbstractFormDelegate {
                    Layout.fillWidth: true
                    visible: sheetDrawer.stop.loadInformation != ""
                        contentItem: ColumnLayout {
                            spacing: Kirigami.Units.mediumSpacing

                            QQC2.Label {
                                text: i18n("Vehicle Load:")
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                Accessible.ignored: true
                            }
                            VehicleLoadIndicator {
                                loadInformation: sheetDrawer.stop.loadInformation
                            }
                        }

                    background: Item{}
                }
                FormCard.FormDelegateSeparator {}
                FormCard.FormButtonDelegate {
                    Layout.fillWidth: true
                    text: i18n("Show location")
                    icon.name: "map-symbolic"
                    onClicked: {
                        sheetDrawer.close()
                        const args = {
                            coordinate: Qt.point(sheetDrawer.stop.stopPoint.longitude, sheetDrawer.stop.stopPoint.latitude),
                            placeName: sheetDrawer.stop.stopPoint.name
                        };
                        if (!sheetDrawer.isDeparture) {
                            args.arrivalPlatformName = sheetDrawer.stop.hasExpectedPlatform ? sheetDrawer.stop.expectedPlatform : sheetDrawer.stop.scheduledPlatform;
                            args.arrivalPlatformMode = PublicTransport.lineModeToPlatformMode(sheetDrawer.stop.route.line.mode);
                            args.arrivalPlatformIfopt = sheetDrawer.stop.stopPoint.identifier("ifopt");
                        }
                        if (!sheetDrawer.isArrival) {
                            args.departurePlatformName = sheetDrawer.stop.hasExpectedPlatform ? sheetDrawer.stop.expectedPlatform : sheetDrawer.stop.scheduledPlatform;
                            args.departurePlatformMode = PublicTransport.lineModeToPlatformMode(sheetDrawer.stop.route.line.mode);
                            args.departurePlatformIfopt = sheetDrawer.stop.stopPoint.identifier("ifopt");
                        }

                        // ensure the map page ends up on top
                        if (applicationWindow().pageStack.layers.depth < 2)
                            applicationWindow().pageStack.push(indoorMapPage, args);
                        else
                            applicationWindow().pageStack.layers.push(indoorMapPage, args);
                        }
                }
                Item {Layout.fillHeight: true}

            }
        }

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
                            MouseArea {
                                anchors.fill: parent
                                scale: 2
                                onClicked: {
                                    sheetDrawer.open()
                                    sheetDrawer.isArrival = false
                                    sheetDrawer.isDeparture = true
                                    sheetDrawer.stop = journeySection.departure
                                }
                            }
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
                                MouseArea {
                                    anchors.fill: parent
                                    scale: 3
                                    onClicked: {
                                        sheetDrawer.open()
                                        sheetDrawer.isArrival = false
                                        sheetDrawer.isDeparture = false
                                        sheetDrawer.stop = model.stopover
                                    }
                                }
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
                            MouseArea {
                                anchors.fill: parent
                                scale: 2
                                onClicked: {
                                    sheetDrawer.open()
                                    sheetDrawer.isArrival = true
                                    sheetDrawer.isDeparture = false
                                    sheetDrawer.stop = journeySection.arrival
                                }
                            }
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
