// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item{
    id: container

    property list<Kirigami.Action> actions
    property bool consistentWidth: false
    property int defaultIndex: 0
    readonly property int selectedIndex: marker.selectedIndex

    Layout.minimumWidth: consistentWidth ? 0 : switchLayout.implicitWidth
    implicitHeight: switchLayout.implicitHeight
    Layout.fillWidth: consistentWidth

    QQC2.ButtonGroup {
        buttons: switchLayout.children
    }

    RowLayout {
        id: switchLayout
        anchors {
            top: container.top
            left: container.left
            right: container.right
        }
        Repeater{
            id: repeater

            model: actions
            delegate: QQC2.ToolButton {
                id: button

                required property var modelData
                required property int index

                Layout.fillWidth: true
                Layout.preferredWidth: consistentWidth ? (container.width/repeater.count)-(switchLayout.spacing/repeater.count-1) : button.implicitWidth
                Layout.minimumHeight: Math.round(Kirigami.Units.gridUnit * 1.5)

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
                        Layout.leftMargin:  icon.visible ? Kirigami.Units.smallSpacing : Kirigami.Units.largeSpacing
                        Layout.fillWidth: true
                    }

                    Kirigami.Icon {
                        id: icon

                        Layout.alignment: Qt.AlignVCenter

                        color: button.checked ? Kirigami.Theme.hoverColor : Kirigami.Theme.textColor
                        visible: button.icon.name
                        source: button.icon.name
                        implicitHeight: label.height
                        implicitWidth: label.height
                        Behavior on color {
                            PropertyAnimation {
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutCubic
                            }
                        }
                    }
                    Item{
                        Layout.alignment: Qt.AlignVCenter

                        implicitWidth: fakeLabel.implicitWidth
                        implicitHeight: fakeLabel.implicitHeight
                        QQC2.Label {
                            id: fakeLabel

                            anchors.centerIn: parent
                            font.bold: true
                            color: Kirigami.Theme.hoverColor

                            opacity: button.checked ? 1 : 0
                            text: button.text
                            Behavior on opacity {
                                PropertyAnimation {
                                    duration: Kirigami.Units.longDuration
                                    easing.type: Easing.InOutCubic
                                }
                            }
                            Accessible.ignored: true
                        }
                        QQC2.Label {
                            id: label

                            anchors.centerIn: parent
                            color: Kirigami.Theme.textColor

                            opacity: button.checked ? 0 : 1
                            text: button.text
                            Behavior on opacity {
                                PropertyAnimation {
                                    duration: Kirigami.Units.longDuration
                                    easing.type: Easing.InOutCubic
                                }
                            }
                            Accessible.ignored: true
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.rightMargin: Kirigami.Units.largeSpacing
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

        property int selectedIndex: container.defaultIndex

        y: switchLayout.y
        z: switchLayout.z - 1
        height: switchLayout.implicitHeight
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
