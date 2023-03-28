/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kopeninghours 1.0
import org.kde.itinerary 1.0

Column {
    id: root
    property var model
    property var mapData

    property var oh: {
        var v = OpeningHoursParser.parse(model.value);
        v.region = root.mapData.regionCode;
        v.timeZone = root.mapData.timeZone;
        v.setLocation(root.mapData.center.y, root.mapData.center.x);
        if (v.error != OpeningHours.NoError) {
            console.log("Opening hours parsing error:", v.error, root.mapData.region, root.mapData.timeZone)
        }
        return v;
    }

    QQC2.Label {
        property var currentInterval: root.oh.interval(new Date())

        id: currentState
        text: intervalModel.currentState // TODO we could update this every minute
        color: {
            switch (currentInterval.state) {
                case Interval.Open: return Kirigami.Theme.positiveTextColor;
                case Interval.Closed: return Kirigami.Theme.negativeTextColor;
                default: return Kirigami.Theme.textColor;
            }
        }
        visible: text !== ""
    }

    Component {
        id: intervalDelegate
        Item {
            id: delegateRoot
            property var dayData: model
            implicitHeight: row.implicitHeight
            Row {
                id: row
                QQC2.Label {
                    text: dayData.shortDayName
                    width: delegateRoot.ListView.view.labelWidth + Kirigami.Units.smallSpacing
                    Component.onCompleted: delegateRoot.ListView.view.labelWidth = Math.max(delegateRoot.ListView.view.labelWidth, implicitWidth)
                    font.bold: dayData.isToday
                }
                Repeater {
                    model: dayData.intervals
                    Rectangle {
                        id: intervalBox
                        property var interval: modelData
                        property var closeColor: Kirigami.Theme.negativeBackgroundColor;
                        color: {
                            switch (interval.state) {
                                case Interval.Open: return Kirigami.Theme.positiveBackgroundColor;
                                case Interval.Closed: return intervalBox.closeColor;
                                case Interval.Unknown: return Kirigami.Theme.neutralBackgroundColor;
                            }
                            return "transparent";
                        }
                        width: {
                            var ratio = (interval.estimatedEnd - interval.begin + interval.dstOffset * 1000) / (24 * 60 * 60 * 1000);
                            return ratio * (delegateRoot.ListView.view.width - delegateRoot.ListView.view.labelWidth - Kirigami.Units.smallSpacing);
                        }
                        height: Kirigami.Units.gridUnit
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: intervalBox.color }
                            GradientStop { position: (interval.end - interval.begin) / (interval.estimatedEnd - interval.begin); color: intervalBox.color }
                            GradientStop { position: 1.0; color: interval.hasOpenEndTime ? intervalBox.closeColor : intervalBox.color }
                        }

                        QQC2.Label {
                            id: commentLabel
                            text: interval.comment
                            anchors.centerIn: parent
                            visible: commentLabel.implicitWidth < intervalBox.width
                            font.italic: true
                        }
                    }
                }
            }
            Rectangle {
                id: nowMarker
                property double position: (Date.now() - dayData.dayBegin) / (24 * 60 * 60 * 1000)
                visible: position >= 0.0 && position < 1.0
                color: Kirigami.Theme.textColor
                width: 2
                height: Kirigami.Units.gridUnit
                x: position * (delegateRoot.ListView.view.width - delegateRoot.ListView.view.labelWidth - Kirigami.Units.smallSpacing)
                    + delegateRoot.ListView.view.labelWidth + Kirigami.Units.smallSpacing
            }
        }
    }

    IntervalModel {
        id: intervalModel
        openingHours: root.oh
        // TODO we could use the layover time here, if available and in the future
        beginDate: intervalModel.beginOfWeek(new Date())
        endDate: new Date(intervalModel.beginDate.getTime() + 7 * 24 * 3600 * 1000)
    }

    FontMetrics {
        id: fm
    }

    ListView {
        id: intervalView
        width: parent.width
        height: contentHeight
        boundsBehavior: Flickable.StopAtBounds
        visible: root.oh.error == OpeningHours.NoError
        model: intervalModel
        delegate: intervalDelegate
        property int labelWidth: 0
        spacing: Kirigami.Units.smallSpacing
        clip: true
        header: Row {
            id: intervalHeader
            property int colCount: (intervalView.width - Kirigami.Units.smallSpacing - intervalView.labelWidth) / fm.advanceWidth(intervalModel.formatTimeColumnHeader(12, 59)) < 8 ? 4 : 8
            property int itemWidth: (intervalHeader.ListView.view.width -  intervalHeader.ListView.view.labelWidth - Kirigami.Units.smallSpacing) / colCount
            x: intervalHeader.ListView.view.labelWidth + Kirigami.Units.smallSpacing + intervalHeader.itemWidth/2
            Repeater {
                // TODO we might need to use less when space constrained horizontally
                model: colCount - 1
                QQC2.Label {
                    text: intervalModel.formatTimeColumnHeader((modelData + 1) * 24/colCount, 0)
                    width: intervalHeader.itemWidth
                    horizontalAlignment: Qt.AlignHCenter
                }
            }
        }
    }

    QQC2.Label {
        id: fallbackLabel
        visible: !intervalView.visible
        text: model.value.replace(/;\s*/g, "\n")
    }
}
