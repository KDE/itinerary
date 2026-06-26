/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2026 Jonah Brüchert  <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard


Kirigami.Page {
    id: root

    title: i18n("Journey Details")

    property alias journeySection: view.journeySection
    property alias departureStopIndex: view.departureStopIndex
    property alias arrivalStopIndex: view.arrivalStopIndex
    property alias showProgress: view.showProgress
    property alias enableMapView: view.enableMapView

    padding: 0

    Kirigami.ColumnView.preventStealing: swipeView.currentIndex === 1  // map

    QQC2.SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.selectedIndex

        onCurrentItemChanged: tabBar.selectedIndex = currentIndex

        JourneySectionView {
            id: view
        }
    }

    Loader {
        id: mapLoader
        active: root.journeySection && (root.journeySection.intermediateStops.length > 0 || !root.journeySection.path.isEmpty)
        asynchronous: true

        sourceComponent: JourneySectionMapView {
            // Hack to prevent SwipeView from allowing to swipe to the map when it is not active
            parent: swipeView
            visible: mapLoader.status == Loader.Ready

            attribution.anchors.bottomMargin: root.width < floatingToolBar.implicitWidth + attribution.implicitWidth * 2
                                              ? Kirigami.Units.largeSpacing * 2 + floatingToolBar.implicitHeight
                                              : Kirigami.Units.largeSpacing

            journeySection: root.journeySection

            MapStopoverInfoSheetDrawer {
                id: sheetDrawer

                parent: root.QQC2.Overlay.overlay
            }

            onStopoverClicked: elem => {
                sheetDrawer.isArrival = elem.isArrival;
                sheetDrawer.isDeparture = elem.isDeparture;
                sheetDrawer.stop = elem.stopover;
                sheetDrawer.popup();
            }

            MapPin {
                iconName: "media-playback-start"
                coordinate {
                    latitude: root.journeySection.stopover(root.departureStopIndex).stopPoint.latitude
                    longitude: root.journeySection.stopover(root.departureStopIndex).stopPoint.longitude
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
                    latitude: root.journeySection.stopover(root.arrivalStopIndex).stopPoint.latitude
                    longitude: root.journeySection.stopover(root.arrivalStopIndex).stopPoint.longitude
                }
                onClicked: {
                    sheetDrawer.isArrival = true;
                    sheetDrawer.isDeparture = false;
                    sheetDrawer.stop = root.journeySection.stopover(root.arrivalStopIndex);
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
                    text: i18n("Journey")
                    checked: swipeView.currentIndex === 0
                },
                Kirigami.Action {
                    text: i18n("Map")
                    enabled: root.journeySection && (root.journeySection.intermediateStops.length > 0 || !root.journeySection.path.isEmpty)
                    checked: swipeView.currentIndex === 1
                }
            ]
        }
    }
}
