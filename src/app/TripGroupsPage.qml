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

    TripGroupEditorDialog {
        id: createTripDialog
        onAccepted: {
            const tgId = TripGroupManager.createEmptyGroup(createTripDialog.tripGroupName);
            root.openTripGroupPage(tgId);
        }
    }

    /** Open trip group page for @p tgId. */
    function openTripGroupPage(tgId: string): TripGroupPage {
        const tg = TripGroupManager.tripGroup(tgId);
        if (tg.name === "")
            return;

        while (applicationWindow().pageStack.depth > 1) {
            applicationWindow().pageStack.pop();
        }

        applicationWindow().pageStack.push(Qt.createComponent('org.kde.itinerary', 'TripGroupPage'), {
            tripGroupId: tgId,
            tripGroup: tg,
        });
        return applicationWindow().pageStack.currentItem;
    }

    /** Open reservation details page for the current reservation. */
    function openCurrentReservation() {
        const tgId = TripGroupManager.tripGroupIdForReservation(TripGroupModel.currentBatchId);
        const page = openTripGroupPage(tgId);
        page.showDetailsPageForReservation(TripGroupModel.currentBatchId);
    }

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
            required property string tripGroupId
            required property bool isSingleDay

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
                    text: {
                        if (delegate.isSingleDay)
                            return delegate.begin.toLocaleDateString();
                        if (delegate.begin > 0)
                            return i18nc("date range", "%1 - %2", delegate.begin.toLocaleDateString(), delegate.end.toLocaleDateString());
                        return i18n("Upcoming");
                    }
                    elide: Text.ElideRight
                    opacity: 0.8

                    Layout.fillWidth: true
                }
            }

            onClicked: root.openTripGroupPage(delegate.tripGroupId)
            Accessible.onPressAction: root.openTripGroupPage(delegate.tripGroupId)
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: tripsView
            visible: tripsView.count === 0
            text: i18n("No trips added yet.")
            helpfulAction: Kirigami.Action {
                text: i18nc("@action:button", "Add trip")
                icon.name: "list-add-symbolic"
                onTriggered: createTripDialog.open();
            }
        }
    }

    Components.FloatingButton {
        id: button
        parent: root.overlay
        visible: tripsView.count > 0
        anchors {
            right: parent.right
            rightMargin: Kirigami.Settings.isMobile ? Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }

        action: Kirigami.Action{
            text: i18nc("@action:button", "Add trip")
            icon.name: "list-add-symbolic"
            onTriggered: createTripDialog.open();
        }
    }

    // HACK we have no reliable way of detecting:
    //  - explicitly going back from a TripGroupPage (Qt::BackButton mouse event bypasses Page.backRequested...)
    //  - this page being shown in the sense that it's not just a transient state while popping/clearing the page stack
    Timer {
        id: pageShownTimer
        interval: 300
        onTriggered: {
            if (applicationWindow().pageStack.currentItem === root && root.visible) {
                ApplicationController.contextTripGroupId = "";
            }
        }
    }
    Connections {
        id: pageStackCon
        target: applicationWindow().pageStack
        function onPageRemoved(page : Item) {
            if (page !== root && applicationWindow().pageStack.currentItem === root && root.visible) {
                pageShownTimer.start();
            }
        }

        // HACK pagePushed is emitted when the push isn't sufficiently complete yet, at a point we cannot do another push yet
        property bool _pendingContextRestore: false
        function onPagePushed(page : Item) {
            if (page === root && ApplicationController.contextTripGroupId !== "")
                pageStackCon._pendingContextRestore = true
        }
        function onCurrentIndexChanged() {
            if (pageStackCon._pendingContextRestore && applicationWindow().pageStack.currentItem === root && root.visible) {
                pageStackCon._pendingContextRestore = false;
                root.openTripGroupPage(ApplicationController.contextTripGroupId);
            }
        }
    }
}
