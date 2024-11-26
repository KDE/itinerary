/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: root
    required property var modelData
    required property int index
    required property int modelLength

    topPadding: 0
    bottomPadding: 0

    background.visible: modelData.mode == JourneySection.PublicTransport || modelData.path.sections.length > 1

    onClicked: {
        if (modelData.mode == JourneySection.PublicTransport) {
            applicationWindow().pageStack.push(journeySectionPage, {
                journeySection: root.modelData,
            });
        } else if (modelData.path.sections.length > 1) {
            applicationWindow().pageStack.push(journeyPathPage, {
                path: root.modelData.path,
            });
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        // top row: departure location and departure time
        RowLayout {
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Item {
                visible: root.modelData.mode !== JourneySection.Walking || root.index === 0
                Layout.preferredWidth: departureLine.width
                Layout.fillHeight: true
                Rectangle {
                    visible: root.index != 0 || root.modelData.mode == JourneySection.Walking
                    height: parent.height
anchors.centerIn: parent
                    Layout.margins: 0
                    color: Kirigami.Theme.disabledTextColor
                    width: Kirigami.Units.smallSpacing / 2
                }

                JourneySectionStopDelegateLineSegment {
                    id: departureLine
                    anchors {
                        topMargin: Kirigami.Units.mediumSpacing
                    }
                    height: parent.height
                    width: implicitWidth
                    lineColor: modelData.route.line.hasColor ? modelData.route.line.color : Kirigami.Theme.textColor
                    isDeparture: true
                    visible:  modelData.mode !== JourneySection.Transfer && modelData.mode !== JourneySection.Walking && modelData.mode !== JourneySection.Walking
                }
            }

            RowLayout {
                Layout.topMargin: Kirigami.Units.mediumSpacing
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                visible: (root.modelData.mode !== JourneySection.Waiting && modelData.mode !== JourneySection.Walking ) || index === 0

                Kirigami.Heading {
                    level: 2
                    text: modelData.from.name
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Kirigami.Heading {
                    id: depTime
                    text: root.modelData.hasExpectedDepartureTime ? Localizer.formatTime(root.modelData, "expectedDepartureTime") : Localizer.formatTime(root.modelData, "scheduledDepartureTime")
                }
            }
        }

        // middle row: mode symbol, transport mode, platform
        RowLayout {
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Item {
                Layout.preferredWidth: departureLine.width
                Layout.fillHeight: true

                Rectangle{
                    height: parent.height
                    anchors.centerIn: parent
                    color: Kirigami.Theme.disabledTextColor
                    width: Math.round(Kirigami.Units.smallSpacing / 2)
                }

                JourneySectionStopDelegateLineSegment {
                    anchors.fill: parent
                    lineColor: modelData.route.line.hasColor ? modelData.route.line.color : Kirigami.Theme.textColor
                    hasStop: false
                    visible: modelData.mode !== JourneySection.Transfer && modelData.mode !== JourneySection.Walking && modelData.mode !== JourneySection.Waiting
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.fillWidth: true

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Layout.fillWidth: true

                    TransportNameControl {
                        iconName: root.modelData.iconName
                        line: root.modelData.route.line
                        mode: root.modelData.mode
                        visible: root.modelData.mode === JourneySection.PublicTransport
                    }

                    QQC2.Label {
                        elide: Text.ElideRight
                        visible: text.length > 0
                        text: switch (modelData.mode) {
                        case JourneySection.PublicTransport:
                            if (modelData.route.direction)
                                return i18n("To %1", modelData.route.direction);
                            return ''
                        case JourneySection.Walking:
                            if (modelData.distance == 0)
                                return i18n("Walk (%1)", Localizer.formatDurationCustom(modelData.duration));
                            return i18n("Walk %1 (%2)", Localizer.formatDistance(modelData.distance), Localizer.formatDurationCustom(modelData.duration));
                        case JourneySection.Transfer:
                            return i18n("Transfer (%1)", Localizer.formatDurationCustom(modelData.duration))
                        case JourneySection.Waiting:
                            return i18n("Wait (%1)", Localizer.formatDurationCustom(modelData.duration))
                        case JourneySection.RentedVehicle:
                            return i18n("%1 %2 (%3)", modelData.rentalVehicle.network.name, Localizer.formatDistance(modelData.distance), Localizer.formatDurationCustom(modelData.duration));
                        case JourneySection.IndividualTransport:
                            return i18n("Drive %1 (%2)", Localizer.formatDistance(modelData.distance), Localizer.formatDuration(modelData.duration));
                        return "???";
                        }

                        Layout.topMargin: modelData.mode !== JourneySection.PublicTransport ? Kirigami.Units.largeSpacing : 0
                        Layout.fillWidth: true
                    }
                }

                QQC2.Label {
                    visible: text.length > 0
                    elide: Text.ElideRight
                    text: modelData.mode === JourneySection.PublicTransport ? Localizer.formatDurationCustom(modelData.duration) : ''

                    Layout.fillWidth: true
                }

                DelayRow {
                    stopover: root.modelData
                    delay: root.modelData.departureDelay
                    originalTime: Localizer.formatTime(root.arrival, "scheduledDepartureTime")
                    visible: root.modelData.hasExpectedDepartureTime
                }

                QQC2.Label {
                    text: i18nc("@info", "Platform %1", modelData.hasExpectedDeparturePlatform ? modelData.expectedDeparturePlatform : modelData.scheduledDeparturePlatform)
                    color: modelData.departurePlatformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    visible: root.modelData.scheduledDeparturePlatform.length > 0
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Repeater {
                        model: modelData.features
                        delegate: KPublicTransport.FeatureIcon {
                            feature: modelData
                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        }
                    }
                }
                QQC2.Label {
                    id: notesLabel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: modelData.notes.join("<br/>")
                    color: Kirigami.Theme.disabledTextColor
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignTop
                    // Doesn't work with RichText.
                    elide: Text.ElideRight
                    maximumLineCount: 3
                    Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount
                    visible: modelData.notes.length > 0
                    font.italic: true
                    clip: implicitHeight > height
                    onLinkActivated: Qt.openUrlExternally(link)

                    SheetDrawer {
                        id: moreNotesSheet
                        parent: applicationWindow().overlay

                        headerItem: Kirigami.Heading {
                            text: journeyTitleLabel.text
                            elide: Qt.ElideRight
                            Layout.fillWidth: true
                            leftPadding: Kirigami.Units.smallSpacing
                            rightPadding: Kirigami.Units.smallSpacing
                        }

                        contentItem: ColumnLayout {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 60
                            Layout.maximumWidth: root.width
                            PublicTransportFeatureList {
                                model: modelData.features
                            }
                            QQC2.Label {
                                Layout.fillWidth: true
                                text: modelData.notes.join("<br/>")
                                textFormat: Text.RichText
                                wrapMode: Text.Wrap
                                onLinkActivated: Qt.openUrlExternally(link)
                                padding: Kirigami.Units.largeSpacing * 2

                            }
                        }
                    }
                }
                Kirigami.LinkButton {
                    Layout.fillHeight: true
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    text: i18nc("@action:button", "Show More…")
                    visible: notesLabel.implicitHeight > notesLabel.Layout.maximumHeight
                    onClicked: {
                        moreNotesSheet.open();
                    }
                }
            }
        }

        // last row: arrival information
        RowLayout {
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            visible: modelData.mode !== JourneySection.Walking || index === modelLength

            Layout.fillWidth: true

            Item {
                Layout.preferredWidth: departureLine.width
                Layout.fillHeight: true

                Rectangle{
                    visible: index !== modelLength || modelData.mode === JourneySection.Walking
                    height: parent.height
                    anchors.centerIn: parent
                    color: Kirigami.Theme.disabledTextColor
                    width: Math.round(Kirigami.Units.smallSpacing / 2)
                }
                JourneySectionStopDelegateLineSegment {
                    anchors.fill: parent
                    anchors.bottomMargin: Kirigami.Units.mediumSpacing
                    lineColor: modelData.route.line.hasColor ? modelData.route.line.color : Kirigami.Theme.textColor
                    isArrival: true
                    visible:  modelData.mode !== JourneySection.Transfer && modelData.mode !== JourneySection.Walking && modelData.mode !== JourneySection.Waiting
                }
            }

            ColumnLayout {
                spacing: 0
                Layout.fillWidth: true

                Kirigami.Separator {
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.smallSpacing

                    Kirigami.Heading {
                        level: 4
                        text: root.modelData.to.name
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Kirigami.Heading {
                        level: 3
                        text: root.modelData.hasExpectedArrivalTime ? Localizer.formatTime(root.modelData, "expectedArrivalTime") : Localizer.formatTime(root.modelData, "scheduledArrivalTime")
                    }
                }

                DelayRow {
                    stopover: root.modelData
                    visible: root.modelData.hasExpectedArrivalTime
                    delay: root.modelData.arrivalDelay
                    originalTime: Localizer.formatTime(root.modelData, "scheduledArrivalTime")
                }

                QQC2.Label {
                    readonly property string platform: modelData.hasExpectedArrivalPlatform ? modelData.expectedArrivalPlatform : modelData.scheduledArrivalPlatform
                    text: i18nc("@info", "Platform %1", platform)
                    visible: platform.length > 0

                    Layout.fillWidth: true
                }
            }
        }
    }
}
