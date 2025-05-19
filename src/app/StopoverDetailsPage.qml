/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.kosmindoormap as KOSM

Kirigami.Page {
    id: root
    title: isArrival ? i18nc("train/bus arrival", "Arrival Information") : i18nc("train/bus departure", "Departure Information")

    required property KPublicTransport.Manager ptMgr
    required property KPublicTransport.stopover stopover

    property bool isArrival: false

    bottomPadding: 0

    KPublicTransport.VehicleLayoutQueryModel {
        id: vehicleModel
        manager: root.ptMgr
        onContentChanged: {
            root.stopover = vehicleModel.stopover;
            tabBar.selectFirstEnabledTab();
        }
    }

    ColumnLayout {
        anchors.fill: parent

        GridLayout {
            id: headerLayout
            Layout.fillWidth: true
            rowSpacing: Kirigami.Units.largeSpacing * 2
            columnSpacing: rowSpacing
            rows: 2
            columns: 2

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.fillWidth: true
                Layout.row: 0
                Layout.column: 0

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    KPublicTransport.TransportNameControl {
                        id: lineLabel
                        line: root.stopover.route.line
                        opacity: loadingSpinner.running ? 0.75 : 1.0

                        QQC2.BusyIndicator {
                            id: loadingSpinner
                            anchors.centerIn: lineLabel
                            running: vehicleModel.loading && root.stopover.hasTripIdentifiers
                        }
                    }

                    Kirigami.Heading {
                        level: 3
                        text: root.stopover.route.direction
                        visible: root.stopover.route.direction.length > 0
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                KPublicTransport.ExpectedTimeLabel {
                    id: expectedTimeLabel
                    stopover: root.stopover
                    delay: root.isArrival ? root.stopover.arrivalDelay : root.stopover.departureDelay
                    scheduledTime: delay != 0 ? Localizer.formatTime(root.stopover, root.isArrival ? "scheduledArrivalTime" : "scheduledDepartureTime") : ""
                    hasExpectedTime: root.isArrival ? root.stopover.hasExpectedArrivalTime : root.stopover.hasExpectedDepartureTime
                    visible: hasExpectedTime || root.stopover.disruptionEffect === KPublicTransport.Disruption.NoService
                }

                // TODO ideally this would not be shown when having a known pre-defined stop point (with the platform info added in the row above)
                // it's crucial when doing vicinity searches though, where the stop points are unknown and can vary
                RowLayout {
                    spacing: 0
                    Layout.fillWidth: true
                    QQC2.Label {
                        id: stopNameLabel
                        text: i18nc("departure stop", "From %1", root.stopover.stopPoint.name)
                        visible: root.stopover.stopPoint.name !== ""
                        elide: Text.ElideMiddle
                        Layout.maximumWidth: root.width - relativeTimeLabel.implicitWidth - platformLabel.implicitWidth - headerLayout.columnSpacing - root.leftPadding - root.rightPadding
                    }
                    QQC2.Label {
                        id: platformLabel
                        text: (stopNameLabel.visible ? ' · ' : '') + i18nc("@info", "Platform %1", root.stopover.hasExpectedPlatform ? root.stopover.expectedPlatform : root.stopover.scheduledPlatform)
                        color: root.stopover.platformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                        visible: root.stopover.hasExpectedPlatform || root.stopover.scheduledPlatform !== ""
                    }
                }
            }

            Kirigami.Heading {
                id: relativeTimeLabel
                Layout.row: 0
                Layout.column: 1
                font.strikeout: root.stopover.disruptionEffect === KPublicTransport.Disruption.NoService

                // TODO once KFormat::formatRelativeDateTime supports narrow formatting, use that instead when within 1h of now
                text: root.isArrival ?
                    Localizer.formatTime(root.stopover, root.stopover.hasExpectedArrivalTime ? "expectedArrivalTime" : "scheduledArrivalTime") :
                    Localizer.formatTime(root.stopover, root.stopover.hasExpectedDepartureTime ? "expectedDepartureTime" : "scheduledDepartureTime")
            }
        }

        QQC2.TabBar {
            id: tabBar
            Layout.fillWidth: true
            QQC2.TabButton {
                text: i18n("Information")
                enabled: infoView.hasContent
            }
            QQC2.TabButton {
                text: i18n("Trip")
                enabled: tripView.journeySection.mode !== KPublicTransport.JourneySection.Invalid
            }
            QQC2.TabButton {
                text: i18n("Trip Map")
                enabled: tripView.journeySection.mode !== KPublicTransport.JourneySection.Invalid
            }
            QQC2.TabButton {
                text: i18n("Vehicle")
                enabled: root.stopover.vehicleLayout.sections.length > 0
            }
            QQC2.TabButton {
                text: i18n("Stop Map")
                enabled: root.stopover.stopPoint.hasCoordinate
            }

            function selectFirstEnabledTab() {
                if (tabBar.itemAt(tabBar.currentIndex).enabled)
                    return;
                for (let i = 0; i < tabBar.count; ++i) {
                    if (tabBar.itemAt(i).enabled) {
                        tabBar.currentIndex = i;
                        break;
                    }
                }
            }

            onCurrentIndexChanged: {
                // position trip view to current stop
                if (tabBar.currentIndex === 1 && tripView.contentY < 0) {
                    tripView.positionViewAtIndex(tripView.stopIndex - 1, ListView.Beginning)
                }
                // trip map view on-demand loading
                if (tabBar.currentIndex === 2 && tripView.journeySection.mode !== KPublicTransport.JourneySection.Invalid && tripMapView.journeySection.mode === KPublicTransport.JourneySection.Invalid) {
                    tripMapView.journeySection = tripView.journeySection;
                }
                // position vehicle layout at the start of the train if not modified yet
                if (tabBar.currentIndex === 3 && vehicleLayout.contentItem.contentY === 0) {
                    let offset = vehicleView.fullLength * vehicleModel.vehicle.platformPositionBegin;
                    offset -= Kirigami.Units.iconSizes.small + Kirigami.Units.largeSpacing; // direction indicator
                    vehicleLayout.contentItem.contentY = offset;
                }
                // indoor map view on-demand loading
                if (tabBar.currentIndex === 4 && stopMapView.mapData.isEmpty && !stopMapView.mapLoader.isLoading && root.stopover.stopPoint.hasCoordinate) {
                    stopMapView.mapLoader.loadForCoordinate(root.stopover.stopPoint.latitude, root.stopover.stopPoint.longitude);
                    root.updateStopMap();
                }
            }
        }

        StackLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: tabBar.currentIndex

            QQC2.ScrollView {
                id: infoPage
                contentWidth: width - effectiveScrollBarWidth
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                KPublicTransport.StopoverInformationView {
                    id: infoView
                    width: infoPage.width - infoPage.effectiveScrollBarWidth
                    stopover: root.stopover
                }
            }

            ListView {
                id: tripView
                clip: true
                property KPublicTransport.journeySection journeySection
                property int stopIndex: -1

                header: JourneySectionStopDelegate {
                    stop: tripView.journeySection.departure
                    isDeparture: true
                    highlight: tripView.stopIndex === 0
                    topPadding: Kirigami.Units.largeSpacing
                }

                delegate: JourneySectionStopDelegate {
                    stop: modelData
                    highlight: tripView.stopIndex === index + 1
                }

                footer: JourneySectionStopDelegate {
                    stop: tripView.journeySection.arrival
                    isArrival: true
                    highlight: tripView.stopIndex === tripView.count + 1
                }

                model: journeySection.intermediateStops
            }

            Item {
                JourneySectionMapView {
                    id: tripMapView
                    journeySection: tripView.journeySection
                    anchors { // hack to remove the horizontal padding, as StackLayout ignores Layout.[left|right]Margin
                        fill: parent
                        leftMargin: -root.leftPadding
                        rightMargin: -root.rightPadding
                    }

                    MapPin {
                        iconName: "media-playback-start" // TODO better icon
                        coordinate {
                            latitude: root.stopover.stopPoint.latitude
                            longitude: root.stopover.stopPoint.longitude
                        }
                        // too little contrast when zoomed out currently
                        // color: tripMapView.lineColor
                        onClicked: {
                            mapStopDrawer.isArrival = false
                            mapStopDrawer.isDeparture = true
                            mapStopDrawer.stop = tripMapView.journeySection.stopover(tripView.stopIndex)
                            mapStopDrawer.open();
                        }
                    }

                    onStopoverClicked: (elem) => {
                        mapStopDrawer.isArrival = elem.isArrival;
                        mapStopDrawer.isDeparture = elem.isDeparture;
                        mapStopDrawer.stop = elem.stopover;
                        mapStopDrawer.open();
                    }
                }
            }

            QQC2.ScrollView {
                id: vehicleLayout
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                KPublicTransport.VehicleLayoutView {
                    id: vehicleView
                    model: vehicleModel
                    width: vehicleLayout.width - vehicleLayout.effectiveScrollBarWidth
                    onVehicleSectionTapped: (section) => {
                        coachDrawer.coach = section;
                        coachDrawer.open();
                    }
                }
            }

            Item {
                KOSM.IndoorMapView {
                    id: stopMapView

                    anchors { // see above
                        fill: parent
                        leftMargin: -root.leftPadding
                        rightMargin: -root.rightPadding
                    }

                    overlaySources: [ platformModel, stopMapView.equipmentModel ]

                    elementInfoModel {
                        allowOnlineContent: Settings.wikimediaOnlineContentEnabled
                        debug: Settings.developmentMode
                    }

                    elementInfoDialog: IndoorMapInfoSheet {
                        model: stopMapView.elementInfoModel
                        regionCode: stopMapView.mapData.regionCode
                        timeZone: stopMapView.mapData.timeZone
                    }

                    KOSM.PlatformModel {
                        id: platformModel
                        mapData: stopMapView.mapData

                        onPlatformIndexChanged: {
                            if (platformModel.departurePlatformRow >= 0) {
                                const idx = platformModel.index(platformModel.departurePlatformRow, 0);
                                stopMapView.view.centerOn(platformModel.data(idx, KOSM.PlatformModel.CoordinateRole), platformModel.data(idx, KOSM.PlatformModel.LevelRole), 19);
                            }
                        }
                    }

                    // TODO realtime elevator status
                }
            }
        }
    }

    VehicleSectionDialog {
        id: coachDrawer
    }

    MapStopoverInfoSheetDrawer {
        id: mapStopDrawer
    }

    function updateStopMap() {
        // TODO platform, times, platform sections
        platformModel.departurePlatform.name = root.stopover.hasExpectedPlatform ? root.stopover.expectedPlatform : root.stopover.scheduledPlatform
        switch (root.stopover.route.line.mode) {
            case KPublicTransport.Line.Metro:
                platformModel.departurePlatform.mode = KOSM.Platform.Subway;
                break;
            case KPublicTransport.Line.Tramway:
                platformModel.departurePlatform.mode = KOSM.Platform.Tram;
                break;
            case KPublicTransport.Line.Bus:
            case KPublicTransport.Line.Coach:
            case KPublicTransport.Line.Shuttle:
                platformModel.departurePlatform.mode = KOSM.Platform.Bus;
                break;
        }
        platformModel.departurePlatform.ifopt = root.stopover.stopPoint.identifier("ifopt")
    }

    Component.onCompleted: {
        let reply = ptMgr.queryTrip({ stopover: root.stopover, backendIds: root.backendIds, downloadAssets: true })
        reply.finished.connect(() => {
            loadingSpinner.running = Qt.binding(() => { return vehicleModel.loading; });
            if (reply.error === KPublicTransport.Reply.NoError) {
                tripView.journeySection = reply.trip;
                tripView.stopIndex = reply.stopoverIndex;
                root.stopover = reply.stopover;
                vehicleModel.request.stopover = reply.stopover;
                vehicleModel.request.backendIds = root.backendIds;
                tabBar.selectFirstEnabledTab();
                root.updateStopMap();
            } else {
                showPassiveNotification(reply.errorString);
            }
            reply.destroy();
        });

        tabBar.selectFirstEnabledTab();
    }
}
