/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.coreaddons as CoreAddons
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KirigamiDelegates
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary


Kirigami.Page {
    id: root
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    title: i18n("Journey Details")

    property KPublicTransport.journeySection journeySection
    property int departureStopIndex: 0
    property int arrivalStopIndex: root.journeySection.intermediateStops.length + 1
    property alias showProgress: sectionModel.showProgress
    property alias enableMapView: mapButton.visible
    default property alias _children: root.children

    Kirigami.ColumnView.preventStealing: view.currentItem.objectName === "sectionMap"

    padding: 0
    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        visible: root.journeySection != undefined

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            KPublicTransport.TransportNameControl {
                journeySection: root.journeySection
            }

            QQC2.Label {
                text: i18nc("Direction of the transport mode", "To %1", root.journeySection.route.direction)
                visible: root.journeySection.route.direction.length > 0
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: i18n("Distance: %1", CoreAddons.Format.formatDistance(journeySection.distance, Settings.distanceFormat))
            visible: root.journeySection.distance > 0
            wrapMode: Text.Wrap

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: i18n("Average Speed: %1", Localizer.formatSpeed(journeySection.distance / journeySection.duration * 3.6, Settings.distanceFormat))
            visible: journeySection.distance > 0 && journeySection.duration > 0
            wrapMode: Text.Wrap

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: i18n("CO₂: %1", Localizer.formatWeight(journeySection.co2Emission))
            visible: journeySection.co2Emission > 0
            wrapMode: Text.Wrap

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            Repeater {
                model: root.journeySection.departureVehicleLayout.combinedFeatures
                delegate: KPublicTransport.FeatureIcon {
                    feature: modelData
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                }
            }

            TapHandler {
                onTapped: moreNotesSheet.open()
            }
        }

        QQC2.Label {
            id: notesLabel

            text: journeySection.notes.join("<br/>")
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignTop
            // Doesn't work with RichText.
            elide: Text.ElideRight
            maximumLineCount: 5
            clip: true
            visible: journeySection.notes.length > 0
            font.italic: true
            onLinkActivated: Qt.openUrlExternally(link)

            Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        Kirigami.LinkButton {
            text: i18nc("@action:button", "Show More…")
            visible: notesLabel.implicitHeight > notesLabel.height
            onClicked: {
                moreNotesSheet.open();
            }

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }
    }

    data: StopoverInformationSheet {
        id: moreNotesSheet
        stopover: root.journeySection.departure
        notes: root.journeySection.notes
    }

    JourneySectionModel {
        id: sectionModel
        journeySection: root.journeySection
    }

    MapStopoverInfoSheetDrawer {
        id: sheetDrawer
        anchors.fill: parent
    }

    contentItem: Item {
        QQC2.SwipeView {
            id: view
            clip: true
            interactive: root.enableMapView
            anchors.fill: parent
            Item {
                id: listPage

                objectName: "sectionList"

                QQC2.ScrollView{
                    id: scrollview
                    anchors.fill: parent

                    QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                    ListView {
                        id: journeyListView
                        clip: true
                        model: sectionModel
                        leftMargin: Kirigami.Units.largeSpacing
                        rightMargin: Kirigami.Units.largeSpacing
                        header: JourneySectionStopDelegate {
                            x: 0
                            topPadding: Kirigami.Units.largeSpacing
                            stop: journeySection.departure
                            isDeparture: true
                            highlight: root.departureStopIndex === 0
                            stopoverPassed: sectionModel.departed
                            visible: root.journeySection != undefined
                            progress: sectionModel.departureProgress
                        }
                        delegate: JourneySectionStopDelegate {
                            stop: model.stopover
                            stopoverPassed: model.stopoverPassed
                            highlight: root.departureStopIndex === index + 1 || root.arrivalStopIndex === index + 1
                            progress: model.progress
                        }
                        footer: ColumnLayout {
                            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                            spacing: Kirigami.Units.smallSpacing

                            JourneySectionStopDelegate {
                                Layout.fillWidth: true
                                stop: journeySection.arrival
                                isArrival: true
                                highlight: root.arrivalStopIndex === root.journeySection.intermediateStops.length + 1
                                stopoverPassed: sectionModel.arrived
                                visible: root.journeySection != undefined
                                progress: sectionModel.arrived ? 1 : 0
                            }
                            // spacer for floating buttons not overlapping the list view
                            Item {
                                height: mapButton.height
                            }
                        }
                    }
                }
            }
            Item {
                id: mapPage

                objectName: "sectionMap"

                JourneySectionMapView {
                    id: map
                    journeySection: root.journeySection
                    anchors.fill: parent

                    onStopoverClicked: (elem) => {
                        sheetDrawer.isArrival = elem.isArrival;
                        sheetDrawer.isDeparture = elem.isDeparture;
                        sheetDrawer.stop = elem.stopover;
                        sheetDrawer.open();
                    }

                    MapPin {
                        iconName: "media-playback-start"
                        coordinate {
                            latitude: root.journeySection.stopover(root.departureStopIndex).stopPoint.latitude
                            longitude: root.journeySection.stopover(root.departureStopIndex).stopPoint.longitude
                        }
                        onClicked: {
                            sheetDrawer.open()
                            sheetDrawer.isArrival = false
                            sheetDrawer.isDeparture = true
                            sheetDrawer.stop = root.journeySection.stopover(root.departureStopIndex)
                        }
                    }
                    MapPin {
                        iconName: "media-playback-stop"
                        coordinate {
                            latitude: root.journeySection.stopover(root.arrivalStopIndex).stopPoint.latitude
                            longitude: root.journeySection.stopover(root.arrivalStopIndex).stopPoint.longitude
                        }
                        onClicked: {
                            sheetDrawer.open()
                            sheetDrawer.isArrival = true
                            sheetDrawer.isDeparture = false
                            sheetDrawer.stop = root.journeySection.stopover(root.arrivalStopIndex)
                        }
                    }
                }
            }
        }
        Components.FloatingButton {
            id: mapButton
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

    Component.onCompleted: {
        if (root.departureStopIndex > 0)
            journeyListView.positionViewAtIndex(root.departureStopIndex - 1, ListView.Beginning)
    }
}
