/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.guiaddons as GuiAddons
import org.kde.solidextras as Solid

/** Shared logic and state for the barcode scan mode.
 *  That is, disable screen locking and switch to full brightness.
 */
QtObject {
    id: controller
    /** Whether or not we are in barcode scan mode. */
    property bool enabled: false
    /** The page showing the barcode.
     *  Scan mode will automatically be disabled when the page is left or moves down in the page stack.
     */
    property var page
    /** Enable full brightness. */
    property bool fullBrightness: true

    function toggle() {
        controller.enabled = !controller.enabled;
    }

    onEnabledChanged: function() {
        console.log("switching barcode scan mode", controller.enabled);
        if (controller.fullBrightness) {
            Solid.BrightnessManager.toggleBrightness();
        }
    }

    readonly property GuiAddons.SystemInhibitor inhibitor: GuiAddons.SystemInhibitor {
        window: QQC2.ApplicationWindow.window
        types: GuiAddons.SystemInhibitor.Idle
        enabled: controller.enabled
        reason: i18n("In barcode scanning mode")
    }

    property var __pageWatcher: Connections {
        target: controller.page
        function onVisibleChanged() {
            controller.enabled = false;
        }
    }

    property var __pageStackWather: Connections {
        target: applicationWindow().pageStack
        function onCurrentItemChanged() {
            if (applicationWindow().pageStack.currentItem != page) {
                controller.enabled = false;
            }
        }
    }

    Component.onDestruction: controller.enabled = false
}
