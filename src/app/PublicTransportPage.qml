// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2026 Jonah Brüchert <jbb@kaidan.im>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kitinerary
import org.kde.itinerary

DetailsPage {
    id: root

    padding: 0

    property Component ticketView
    property alias swipeView: view
    property alias banner: banner
    property alias showBarcodeScanButton: scanModeButton.visible
    property int departureStopIndex: 0
    property int arrivalStopIndex: root.controller.journey.intermediateStops.length + 1

    Kirigami.ColumnView.preventStealing: swipeView.currentIndex === 2  // map

    data: BarcodeScanModeButton {
        id: scanModeButton
        page: root
        visible: false

        anchors.rightMargin: Kirigami.Units.largeSpacing
                             + (scrollView.QQC2.ScrollBar && scrollView.QQC2.ScrollBar.vertical ? scrollView.QQC2.ScrollBar.vertical.width : 0)
        anchors.bottomMargin: Kirigami.Units.largeSpacing + (root.width < 500 ? (floatingToolBar.implicitHeight + Kirigami.Units.largeSpacing) : 0)
    }

    header: ColumnLayout {
        KirigamiComponents.Banner {
            id: banner

            showCloseButton: true
            visible: false
        }
    }

    QQC2.SwipeView {
        id: view
        anchors.fill: parent
        currentIndex: tabBar.selectedIndex

        onCurrentItemChanged: tabBar.selectedIndex = currentIndex

        Item {
            QQC2.ScrollView {
                id: scrollView
                anchors.fill: parent
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                contentWidth: availableWidth

                ColumnLayout {
                    width: scrollView.contentWidth

                    Item {
                        implicitHeight: Kirigami.Units.gridUnit
                    }

                    Loader {
                        id: ticketViewLoader

                        Layout.fillWidth: true

                        active: true
                        asynchronous: true
                        sourceComponent: root.ticketView
                        visible: status == Loader.Ready
                    }

                    // spacer for the floating buttons
                    // also makes sure the bottom-most actions can still be reached despite the floating toolbar
                    Item {
                        implicitHeight: (scanModeButton.visible && root.width < Kirigami.Units.gridUnit * 30 + scanModeButton.width * 2 ? scanModeButton.height : 0)
                            + Kirigami.Units.largeSpacing * 2 + floatingToolBar.implicitHeight
                    }
                }
            }

            QQC2.BusyIndicator {
                anchors.centerIn: parent
                running: ticketViewLoader.status === Loader.Loading
                visible: running
            }
        }
    }

    Loader {
        id: sectionDetailsLoader
        active: root.controller.journey && (root.controller.journey.from.name && root.controller.journey.to.name || root.controller.journey.intermediateStops.length > 0 || !root.controller.journey.path.isEmpty)
        asynchronous: true
        sourceComponent: JourneySectionView {
            // Hack to prevent SwipeView from allowing to swipe to the details while not active
            parent: view
            visible: sectionDetailsLoader.status == Loader.Ready

            journeySection: root.controller.trip
            departureStopIndex: root.controller.tripDepartureIndex
            arrivalStopIndex: root.controller.tripArrivalIndex
            showProgress: root.controller.isCurrent
        }
    }

    Loader {
        id: mapLoader
        active: root.controller.journey && (root.controller.journey.intermediateStops.length > 0 || !root.controller.journey.path.isEmpty)
        asynchronous: true

        sourceComponent: JourneySectionMapView {
            // Hack to prevent SwipeView from allowing to swipe to the map when it is not active
            parent: view
            visible: mapLoader.status == Loader.Ready

            attribution.anchors.bottomMargin: root.width < floatingToolBar.implicitWidth + attribution.implicitWidth * 2
                                              ? Kirigami.Units.largeSpacing * 2 + floatingToolBar.implicitHeight
                                              : Kirigami.Units.largeSpacing

            id: map
            journeySection: root.controller.trip

            data: [
                MapStopoverInfoSheetDrawer {
                    id: sheetDrawer

                    parent: root.QQC2.Overlay.overlay
                }
            ]

            onStopoverClicked: elem => {
                sheetDrawer.isArrival = elem.isArrival;
                sheetDrawer.isDeparture = elem.isDeparture;
                sheetDrawer.stop = elem.stopover;
                sheetDrawer.popup();
            }

            MapPin {
                iconName: "media-playback-start"
                coordinate {
                    latitude: root.controller.journey.stopover(root.departureStopIndex).stopPoint.latitude
                    longitude: root.controller.journey.stopover(root.departureStopIndex).stopPoint.longitude
                }
                onClicked: {
                    sheetDrawer.isArrival = false;
                    sheetDrawer.isDeparture = true;
                    sheetDrawer.stop = root.journeySection.stopover(root.departureStopIndex);
                    sheetDrawer.popup();
                }
            }
            MapPin {
                iconName: "media-playback-stop"
                coordinate {
                    latitude: root.controller.journey.stopover(root.arrivalStopIndex).stopPoint.latitude
                    longitude: root.controller.journey.stopover(root.arrivalStopIndex).stopPoint.longitude
                }
                onClicked: {
                    sheetDrawer.isArrival = true;
                    sheetDrawer.isDeparture = false;
                    sheetDrawer.stop = root.controller.journey.stopover(root.arrivalStopIndex);
                    sheetDrawer.popup();
                }
            }
        }
    }

    KirigamiComponents.FloatingToolBar {
        id: floatingToolBar

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottomMargin: Kirigami.Units.largeSpacing
        anchors.leftMargin: root.width / 2 - 150
        anchors.rightMargin: root.width / 2 - 150

        leftPadding: 5
        rightPadding: 5

        contentItem: KirigamiComponents.RadioSelector {
            id: tabBar

            consistentWidth: true

            actions: [
                Kirigami.Action {
                    text: i18n("Ticket")
                    checked: view.currentIndex === 0
                },
                Kirigami.Action {
                    text: i18n("Journey")
                    enabled: root.controller.journey && (root.controller.journey.from.name && root.controller.journey.to.name || root.controller.journey.intermediateStops.length > 0 || !root.controller.journey.path.isEmpty)
                    checked: view.currentIndex === 1
                },
                Kirigami.Action {
                    text: i18n("Map")
                    enabled: root.controller.journey && (root.controller.journey.intermediateStops.length > 0 || !root.controller.journey.path.isEmpty)
                    checked: view.currentIndex === 2
                }
            ]
        }
    }
}
