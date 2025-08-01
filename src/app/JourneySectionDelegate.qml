/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.coreaddons as CoreAddons
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: root
    required property KPublicTransport.journeySection modelData
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
                    y: Kirigami.Units.mediumSpacing + Kirigami.Units.smallSpacing
                    height: parent.height - y
                    width: implicitWidth
                    lineColor: root.modelData.route.line.hasColor ? root.modelData.route.line.color : Kirigami.Theme.textColor
                    isDeparture: true
                    visible:  root.modelData.mode !== JourneySection.Transfer && root.modelData.mode !== JourneySection.Walking && root.modelData.mode !== JourneySection.Walking
                }
            }

            RowLayout {
                Layout.topMargin: Kirigami.Units.mediumSpacing
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                visible: (root.modelData.mode !== JourneySection.Waiting && root.modelData.mode !== JourneySection.Walking ) || root.index === 0

                Kirigami.Heading {
                    level: 2
                    text: root.modelData.from.name
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
                    lineColor: root.modelData.route.line.hasColor ? root.modelData.route.line.color : Kirigami.Theme.textColor
                    hasStop: false
                    visible: root.modelData.mode !== JourneySection.Transfer && root.modelData.mode !== JourneySection.Walking && root.modelData.mode !== JourneySection.Waiting
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.fillWidth: true

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Layout.fillWidth: true

                    KPublicTransport.TransportNameControl {
                        journeySection: root.modelData
                        visible: root.modelData.mode === JourneySection.PublicTransport
                    }

                    QQC2.Label {
                        elide: Text.ElideRight
                        visible: text.length > 0
                        text: switch (root.modelData.mode) {
                        case JourneySection.PublicTransport:
                            if (root.modelData.route.direction)
                                return i18n("To %1", root.modelData.route.direction);
                            return ''
                        case JourneySection.Walking:
                            if (modelData.distance == 0)
                                return i18n("Walk (%1)", Localizer.formatDuration(root.modelData.duration));
                            return i18n("Walk %1 (%2)", CoreAddons.Format.formatDistance(root.modelData.distance, Settings.distanceFormat), Localizer.formatDuration(root.modelData.duration));
                        case JourneySection.Transfer:
                            return i18n("Transfer (%1)", Localizer.formatDuration(root.modelData.duration))
                        case JourneySection.Waiting:
                            return i18n("Wait (%1)", Localizer.formatDuration(root.modelData.duration))
                        case JourneySection.RentedVehicle:
                            return i18n("%1 %2 (%3)", root.modelData.rentalVehicle.network.name, CoreAddons.Format.formatDistance(root.modelData.distance, Settings.distanceFormat), Localizer.formatDuration(root.modelData.duration));
                        case JourneySection.IndividualTransport:
                            return i18n("Drive %1 (%2)", CoreAddons.Format.formatDistance(root.modelData.distance, Settings.distanceFormat), Localizer.formatDuration(root.modelData.duration));
                        return "???";
                        }

                        Layout.topMargin: root.modelData.mode !== JourneySection.PublicTransport ? Kirigami.Units.largeSpacing : 0
                        Layout.fillWidth: true
                    }
                }

                QQC2.Label {
                    visible: text.length > 0
                    elide: Text.ElideRight
                    text: root.modelData.mode === JourneySection.PublicTransport ? Localizer.formatDuration(root.modelData.duration) : ''

                    Layout.fillWidth: true
                }

                KPublicTransport.ExpectedTimeLabel {
                    stopover: root.modelData.departure
                    delay: root.modelData.departureDelay
                    scheduledTime: delayed ? Localizer.formatTime(root.modelData, "scheduledDepartureTime") : ""
                    hasExpectedTime: root.modelData.hasExpectedDepartureTime
                    visible: root.modelData.mode !== JourneySection.Walking && (root.modelData.hasExpectedDepartureTime || root.modelData.disruptionEffect === KPublicTransport.Disruption.NoService)
                }

                QQC2.Label {
                    text: i18nc("@info", "Platform %1", root.modelData.hasExpectedDeparturePlatform ? root.modelData.expectedDeparturePlatform : root.modelData.scheduledDeparturePlatform)
                    color: root.modelData.departurePlatformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    visible: root.modelData.scheduledDeparturePlatform.length > 0
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Repeater {
                        model: root.modelData.features
                        delegate: KPublicTransport.FeatureIcon {
                            required property KPublicTransport.feature modelData
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
                    text: root.modelData.notes.join("<br/>")
                    color: Kirigami.Theme.disabledTextColor
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignTop
                    // Doesn't work with RichText.
                    elide: Text.ElideRight
                    maximumLineCount: 3
                    Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount
                    visible: root.modelData.notes.length > 0
                    font.italic: true
                    clip: implicitHeight > height
                    onLinkActivated: Qt.openUrlExternally(link)

                    SheetDrawer {
                        id: moreNotesSheet
                        parent: applicationWindow().overlay

                        headerItem: Kirigami.Heading {
                            text: root.modelData.label
                            elide: Qt.ElideRight
                            Layout.fillWidth: true
                            leftPadding: Kirigami.Units.smallSpacing
                            rightPadding: Kirigami.Units.smallSpacing
                        }

                        contentItem: ColumnLayout {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 60
                            Layout.maximumWidth: root.width
                            Repeater {
                                model: root.modelData.features
                                delegate: KPublicTransport.FeatureDelegate {
                                    Layout.leftMargin: Kirigami.Units.largeSpacing
                                    required property KPublicTransport.feature modelData
                                    feature: modelData
                                }
                            }
                            QQC2.Label {
                                Layout.fillWidth: true
                                text: root.modelData.notes.join("<br/>")
                                textFormat: Text.RichText
                                wrapMode: Text.Wrap
                                onLinkActivated: (link) => { Qt.openUrlExternally(link); }
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
            visible: root.modelData.mode !== JourneySection.Walking || root.index === root.modelLength

            Layout.fillWidth: true

            Item {
                Layout.preferredWidth: departureLine.width
                Layout.fillHeight: true

                Rectangle{
                    visible: root.index !== root.modelLength || root.modelData.mode === JourneySection.Walking
                    height: parent.height
                    anchors.centerIn: parent
                    color: Kirigami.Theme.disabledTextColor
                    width: Math.round(Kirigami.Units.smallSpacing / 2)
                }
                JourneySectionStopDelegateLineSegment {
                    anchors {
                        top: parent.top
                        horizontalCenter: parent.horizontalCenter
                    }
                    lineColor: root.modelData.route.line.hasColor ? root.modelData.route.line.color : Kirigami.Theme.textColor
                    isArrival: true
                    visible:  root.modelData.mode !== JourneySection.Transfer && root.modelData.mode !== JourneySection.Walking && root.modelData.mode !== JourneySection.Waiting
                    height: arrivalStopRow.height + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
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
                    id: arrivalStopRow
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

                KPublicTransport.ExpectedTimeLabel {
                    stopover: root.modelData.arrival
                    visible: root.modelData.mode !== JourneySection.Walking && (root.modelData.hasExpectedArrivalTime || root.modelData.disruptionEffect === KPublicTransport.Disruption.NoService)
                    delay: root.modelData.arrivalDelay
                    scheduledTime: delayed ? Localizer.formatTime(root.modelData, "scheduledArrivalTime") : ""
                    hasExpectedTime: root.modelData.hasExpectedArrivalTime
                }

                QQC2.Label {
                    readonly property string platform: root.modelData.hasExpectedArrivalPlatform ? root.modelData.expectedArrivalPlatform : root.modelData.scheduledArrivalPlatform
                    text: i18nc("@info", "Platform %1", platform)
                    visible: platform.length > 0

                    Layout.fillWidth: true
                }
            }
        }
    }
}
