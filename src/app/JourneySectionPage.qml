/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("Journey Details")
    property var journeySection
    property alias showProgress: sectionModel.showProgress
    default property alias _children: root.children

    header: ColumnLayout {
        spacing: 0
        visible: root.journeySection != undefined

        GridLayout {
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rows: 4
            rowSpacing: 0

            Layout.margins: Kirigami.Units.gridUnit

            Kirigami.Icon {
                Layout.rowSpan: 4
                Layout.alignment: Qt.AlignTop
                Layout.rightMargin: Kirigami.Units.largeSpacing
                id: icon
                source: PublicTransport.lineIcon(journeySection.route.line)
                width: height
                height: Kirigami.Units.iconSizes.large
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
                text: i18n("CO₂: %1", Localizer.formatWeight(journeySection.co2Emission))
                visible: journeySection.co2Emission > 0
            }

            QQC2.Label {
                id: notesLabel
                Layout.row: 4
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
                Layout.row: 5
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
