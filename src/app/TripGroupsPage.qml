// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title:window", "Trips")

    ListView {
        id: tripsView

        section {
            delegate: Kirigami.ListSectionHeader {
                required property int section
                width: tripsView.width

                text: switch (section) {
                case TripGroupModel.Past:
                    return i18nc("List section header", "Past trips");
                case TripGroupModel.Future:
                    return i18nc("List section header", "Future trips");
                case TripGroupModel.Current:
                    return i18nc("List section header", "Current trip");
                }
            }
            property: "position"
        }

        model: TripGroupModel

        delegate: Delegates.RoundedItemDelegate {
            id: delegate

            required property int index
            required property string name
            required property date begin
            required property date end
            required property var tripGroup

            text: name

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    level: 4
                    text: delegate.text
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }

                QQC2.Label {
                    text: delegate.begin.toLocaleDateString() + ' - ' + delegate.end.toLocaleDateString()
                    elide: Text.ElideRight
                    opacity: 0.8

                    Layout.fillWidth: true
                }
            }

            onClicked: applicationWindow().pageStack.push(Qt.createComponent('org.kde.itinerary', 'TripGroupPage'), {
                tripGroup: delegate.tripGroup,
            });
        }
    }

    Components.FloatingButton {
        id: button
        parent: root.overlay
        anchors {
            right: parent.right
            rightMargin: Kirigami.Settings.isMobile ? Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }

        action: Kirigami.Action{
            text: i18nc("@action:button", "Add trip")
            icon.name: "list-add-symbolic"
            onTriggered: {
                const editorComponent = Qt.createComponent('org.kde.itinerary', 'TripGroupEditorDialog');
                const editor = editorComponent.createObject(applicationWindow(), {
                    mode: TripGroupEditorDialog.Add,
                });
                editor.open();
            }
        }
    }
}
