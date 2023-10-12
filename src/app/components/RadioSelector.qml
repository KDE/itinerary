// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.19 as Kirigami

RowLayout {
    id: root

    property list<Kirigami.Action> actions
    property bool consistentWidth: false
    property int defaultIndex: 0
    readonly property int selectedIndex: marker.selectedIndex

    Item{
        id: container

        Layout.minimumWidth: consistentWidth ? 0 : switchLayout.implicitWidth
        height: Math.round(Kirigami.Units.gridUnit * 1.5)
        Layout.fillHeight: true
        Layout.fillWidth: consistentWidth

        QQC2.ButtonGroup {
            buttons: switchLayout.children
        }

        RowLayout {
            id: switchLayout

            anchors.fill: parent
            Layout.fillWidth: true
            Repeater{
                id: repeater

                model: actions
                delegate: QQC2.ToolButton {
                    id: button

                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    Layout.preferredWidth: consistentWidth ? (root.width/repeater.count)-(switchLayout.spacing/repeater.count-1) : button.implicitWidth

                    font.bold: checked
                    checkable: true
                    text: modelData.text
                    icon.name: modelData.icon.name

                    background: Rectangle{
                        anchors.fill: button

                        radius: height/2
                        color: "transparent"
                        border.color: Kirigami.Theme.disabledTextColor
                        opacity: button.hovered ? 0.3 : 0

                        Behavior on opacity {
                            PropertyAnimation {
                                duration: Kirigami.Units.shortDuration
                                easing.type: Easing.InOutCubic
                            }
                        }
                    }

                    contentItem: RowLayout{
                        Item {
                            Layout.leftMargin: Kirigami.Units.smallSpacing
                            Layout.fillWidth: true
                        }

                        Kirigami.Icon {
                            id: icon

                            Layout.topMargin: (container.height-label.height)/2
                            Layout.bottomMargin: (container.height-label.height)/2

                            visible: button.icon.name
                            source: button.icon.name
                            implicitHeight: label.height
                            implicitWidth: label.height
                        }

                        QQC2.Label {
                            id: label

                            Layout.topMargin: (container.height-label.height)/2
                            Layout.bottomMargin: (container.height-label.height)/2

                            color: button.checked ? Kirigami.Theme.hoverColor : Kirigami.Theme.textColor
                            text: button.text
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.rightMargin: Kirigami.Units.smallSpacing
                        }
                    }

                    onClicked: {
                        marker.width = Qt.binding(function() { return width })
                        marker.x = Qt.binding(function() { return x })
                        modelData.triggered()
                        marker.selectedIndex = index

                    }
                    Component.onCompleted: if (index === defaultIndex ) {
                        marker.width = Qt.binding(function() { return width })
                        marker.x = Qt.binding(function() { return x })
                        button.checked = true
                    }
                }
            }
        }

        Rectangle {
            id: marker

            property int selectedIndex: root.defaultIndex

            y: switchLayout.y
            z: switchLayout.z - 1
            height: container.height
            radius: height/2
            border.width: 1
            border.color: Kirigami.ColorUtils.linearInterpolation(
                              Kirigami.Theme.hoverColor,
                              "transparent", 0.4)
            color: Kirigami.ColorUtils.linearInterpolation(
                       Kirigami.Theme.hoverColor,
                       "transparent", 0.9)

            Behavior on x {
                PropertyAnimation {
                    id: x_anim

                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutCubic
                }
            }

            Behavior on width {
                PropertyAnimation {
                    id: width_anim

                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
