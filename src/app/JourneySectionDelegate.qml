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

    contentItem: GridLayout {
        rowSpacing: 0
        columns: 3

        // top row: departure time, departure location, departure platform
        Item {
            visible: modelData.mode !== JourneySection.Walking || index === 0
            width: departureLine.width
            Layout.fillHeight: true
            Layout.row: 0
            Layout.column: 0
            Rectangle{
                visible: index != 0 || modelData.mode == JourneySection.Walking
                height: parent.height
                anchors.centerIn: parent
                Layout.margins: 0
                color: Kirigami.Theme.disabledTextColor
                width: Kirigami.Units.smallSpacing / 2
            }

            JourneySectionStopDelegateLineSegment {
                id: departureLine
                anchors.topMargin: Kirigami.Units.mediumSpacing
                anchors.fill: parent
                lineColor: modelData.route.line.hasColor ? modelData.route.line.color : Kirigami.Theme.textColor
                isDeparture: true
                visible:  modelData.mode !== JourneySection.Transfer && modelData.mode !== JourneySection.Walking && modelData.mode !== JourneySection.Walking
            }
        }
        RowLayout {
            Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
            Layout.topMargin: Kirigami.Units.mediumSpacing
            Layout.row: 0
            Layout.column: 1
            visible: ( root.modelData.mode !== JourneySection.Waiting && modelData.mode !== JourneySection.Walking ) || index === 0


            QQC2.Label {
                id: depTime
                text: Localizer.formatTime(root.modelData, "scheduledDepartureTime")
            }
            QQC2.Label {
                text: {
                    if (root.modelData.disruptionEffect === Disruption.NoService)
                        return i18nc("a train/bus journey canceled by its operator", "Canceled");
                    return (root.modelData.departureDelay >= 0 ? "+" : "") + root.modelData.departureDelay;
                }
                color: {
                    if (root.modelData.departureDelay > 1 || root.modelData.disruptionEffect === Disruption.NoService)
                        return Kirigami.Theme.negativeTextColor;
                    return Kirigami.Theme.positiveTextColor;
                }
                visible: root.modelData.hasExpectedDepartureTime || root.modelData.disruptionEffect === Disruption.NoService
            }
        }
        RowLayout {
            Layout.topMargin: Kirigami.Units.mediumSpacing
            Layout.row: 0
            Layout.column: 2
            visible: ( root.modelData.mode !== JourneySection.Waiting && modelData.mode !== JourneySection.Walking ) || index === 0

            QQC2.Label {
                text: modelData.from.name
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            QQC2.Label {
                text: modelData.hasExpectedDeparturePlatform ? modelData.expectedDeparturePlatform : modelData.scheduledDeparturePlatform
                color: modelData.departurePlatformChanged ? Kirigami.Theme.negativeTextColor
                    : modelData.hasExpectedDeparturePlatform ? Kirigami.Theme.positiveTextColor
                    : Kirigami.Theme.textColor
                visible: root.modelData.scheduledDeparturePlatform.length > 0
            }
        }
        // middle row: mode symbol, transport mode, duration
        Item {
            width: departureLine.width
            Layout.row: 1
            Layout.column: 0
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
        Item{
            Layout.row: 1
            Layout.column: 1
            Layout.preferredWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
            implicitHeight: Kirigami.Units.iconSizes.smallMedium

            Rectangle {
                color: (root.modelData.route.line.hasColor && !modelData.route.line.hasLogo && !modelData.route.line.hasModeLogo) ? modelData.route.line.color : "transparent"
                implicitHeight: parent.height
                implicitWidth: modeIcon.width
                anchors.centerIn: parent
                radius: Kirigami.Units.smallSpacing

                TransportIcon {
                    id: modeIcon
                    anchors.centerIn: parent
                    source: modelData.iconName
                    color: modelData.route.line.hasTextColor ? modelData.route.line.textColor : Kirigami.Theme.textColor
                    width: !isMask ? implicitWidth : height
                    height: parent.height
                    isMask: modelData.mode != JourneySection.PublicTransport || (!modelData.route.line.hasLogo && !modelData.route.line.hasModeLogo)
                }
            }
        }
        RowLayout {
            Layout.row: 1
            Layout.column: 2
            Layout.topMargin: modelData.mode === JourneySection.Walking ? Kirigami.Units.largeSpacing * 3 : 0
            Layout.bottomMargin: modelData.mode === JourneySection.Walking? Kirigami.Units.largeSpacing * 3 : 0

            Layout.fillWidth: true
            QQC2.Label {
                id: journeyTitleLabel
                Layout.fillWidth: true
                text: {
                    switch (modelData.mode) {
                    case JourneySection.PublicTransport:
                    {
                        var l = ""

                        if (modelData.route.line.modeString) {
                            l += modelData.route.line.modeString + " "
                        }

                        l += modelData.route.line.name;

                        if (modelData.route.direction)
                            return i18n("%1 to %2 (%3)", l, modelData.route.direction, Localizer.formatDuration(modelData.duration));
                        return i18n("%1 (%2)", l, Localizer.formatDuration(modelData.duration));
                    }
                    case JourneySection.Walking:
                        if (modelData.distance == 0)
                            return i18n("Walk (%1)", Localizer.formatDuration(modelData.duration));
                        return i18n("Walk %1 (%2)", Localizer.formatDistance(modelData.distance), Localizer.formatDuration(modelData.duration));
                    case JourneySection.Transfer:
                        return i18n("Transfer (%1)", Localizer.formatDuration(modelData.duration))
                    case JourneySection.Waiting:
                        return i18n("Wait (%1)", Localizer.formatDuration(modelData.duration))
                    case JourneySection.RentedVehicle:
                        return i18n("%1 %2 (%3)", modelData.rentalVehicle.network.name, Localizer.formatDistance(modelData.distance), Localizer.formatDuration(modelData.duration));
                    case JourneySection.IndividualTransport:
                        return i18n("Drive %1 (%2)", Localizer.formatDistance(modelData.distance), Localizer.formatDuration(modelData.duration));
                    return "???";
                }}
                color: PublicTransport.warnAboutSection(modelData) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                elide: Text.ElideMiddle
            }
            KPublicTransport.OccupancyIndicator {
                occupancy: PublicTransport.maximumOccupancy(modelData.loadInformation)
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
            }
        }
        // optional middle row: notes
        Item {
            visible: modelData.notes.length > 0 || modelData.features.length > 0
            width: departureLine.width
            Layout.row: 2
            Layout.column: 0
            Layout.fillHeight: true
            Rectangle{
                height: parent.height
                anchors.centerIn: parent
                color: Kirigami.Theme.disabledTextColor
                width: Kirigami.Units.smallSpacing / 2
            }
            JourneySectionStopDelegateLineSegment {
                anchors.fill: parent
                lineColor: modelData.route.line.hasColor ? modelData.route.line.color : Kirigami.Theme.textColor
                hasStop: false
                visible: !(modelData.mode == JourneySection.Transfer || modelData.mode == JourneySection.Walking)
            }
        }
        Item {
            visible: modelData.notes.length > 0 || modelData.features.length > 0
            Layout.row: 2
            Layout.column: 1
            Layout.preferredWidth: Kirigami.Units.largeSpacing
            Layout.fillHeight: true

        }
        ColumnLayout {
            visible: modelData.notes.length > 0 || modelData.features.length > 0
            Layout.row: 2
            Layout.column: 2
            Layout.fillWidth: true
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
        // last row: arrival information
        Item {
            Layout.row: 3
            Layout.column: 0
            Layout.preferredWidth: departureLine.width
            Layout.fillHeight: true
            visible: modelData.mode !== JourneySection.Walking || index === modelLength

            Rectangle{
                visible: index !== modelLength || modelData.mode === JourneySection.Walking
                height: parent.height
                anchors.centerIn: parent
                Layout.margins: 0
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
        RowLayout {
            Layout.row: 3
            Layout.column: 1
            Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
            Layout.bottomMargin: Kirigami.Units.mediumSpacing
            visible: ( modelData.mode !== JourneySection.Waiting && modelData.mode !== JourneySection.Walking) || index === modelLength

            QQC2.Label {
                text: Localizer.formatTime(modelData, "scheduledArrivalTime")
            }
            QQC2.Label {
                text: (modelData.arrivalDelay >= 0 ? "+" : "") + modelData.arrivalDelay
                color: modelData.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: modelData.hasExpectedArrivalTime
            }
        }
        RowLayout {
            Layout.row: 3
            Layout.column: 2
            Layout.bottomMargin: Kirigami.Units.mediumSpacing
            visible: ( modelData.mode !== JourneySection.Waiting && modelData.mode !== JourneySection.Walking ) || index === modelLength

            QQC2.Label {
                text: modelData.to.name
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            QQC2.Label {
                text: modelData.hasExpectedArrivalPlatform ? modelData.expectedArrivalPlatform : modelData.scheduledArrivalPlatform
                color: modelData.arrivalPlatformChanged ? Kirigami.Theme.negativeTextColor
                    : modelData.hasExpectedArrivalPlatform ? Kirigami.Theme.positiveTextColor
                    : Kirigami.Theme.textColor
                visible: modelData.scheduledArrivalPlatform !== ""
            }
        }
    }
}
