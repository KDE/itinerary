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

Item {
    id: root

    property KPublicTransport.journeySection journeySection
    property int departureStopIndex: 0
    property int arrivalStopIndex: root.journeySection.intermediateStops.length + 1
    property alias showProgress: sectionModel.showProgress
    property bool enableMapView
    default property alias _children: root.children

    data: [
        StopoverInformationSheet {
            id: moreNotesSheet
            stopover: root.journeySection.departure
            notes: root.journeySection.notes
            journeySection: root.journeySection
        },
        JourneySectionModel {
            id: sectionModel
            journeySection: root.journeySection
        },
        MapStopoverInfoSheetDrawer {
            id: sheetDrawer

            parent: root.QQC2.Overlay.overlay
        }
    ]

    ColumnLayout {
        id: header

        width: parent.width

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
            visible: notesLabel.implicitHeight > notesLabel.height || moreNotesSheet.hasContent
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

    QQC2.ScrollView {
        id: scrollview

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            anchors.fill: parent
        }

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ListView {
            id: journeyListView
            clip: true
            model: sectionModel
            leftMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

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
                width: ListView.view && ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
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

                Item {
                    implicitHeight: 50
                }
            }
        }
    }

    Component.onCompleted: {
        if (root.departureStopIndex > 0)
            journeyListView.positionViewAtIndex(root.departureStopIndex - 1, ListView.Beginning);
    }
}
