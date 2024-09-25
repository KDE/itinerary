// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item{
    id: root

    property int cornerRadius: Kirigami.Units.cornerRadius

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
            top: root.top
            left: root.left
            right: root.right
        }
        Repeater{
            id: repeater

            model: actions
            delegate: QQC2.ToolButton {
                id: button

                required property var modelData
                required property int index

                Layout.fillWidth: true
                Layout.preferredWidth: consistentWidth ? (root.width/repeater.count)-(switchLayout.spacing/repeater.count-1) : button.implicitWidth
                Layout.minimumHeight: Math.round(Kirigami.Units.gridUnit * 1.5)

                checkable: true
                text: modelData.text
                icon.name: modelData.icon.name

                background: Rectangle{
                    anchors.fill: button

                    radius: root.cornerRadius
                    color: Kirigami.Theme.textColor
                    opacity: Kirigami.Settings.hasTransientTouchInput ? 0 : ( button.hovered ? 0.1 : 0)


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
                        opacity: button.checked ? 1 : 0.7
                        color: Kirigami.Theme.textColor
                        visible: button.icon.name
                        source: button.icon.name
                        implicitHeight: label.height
                        implicitWidth: label.height
                        Behavior on opacity {
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
                            color: Kirigami.Theme.textColor

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

                            opacity: button.checked ? 0 : 0.7
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

    Kirigami.ShadowedRectangle {
        id: marker

        property int selectedIndex: root.defaultIndex

        y: switchLayout.y
        z: switchLayout.z - 1
        height: switchLayout.implicitHeight
        radius: root.cornerRadius
        color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8)
        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.5)
        }
        shadow {
            size: 7
            yOffset: 3
            color: Qt.rgba(0, 0, 0, 0.15)
        }
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
