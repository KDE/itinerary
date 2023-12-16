/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtLocation as QtLocation

/** QtLocation map view with standard intercation settings. */
QtLocation.Map {
    id: map
    plugin: applicationWindow().osmPlugin()
    gesture.acceptedGestures: QtLocation.MapGestureArea.PinchGesture | QtLocation.MapGestureArea.PanGesture
    gesture.preventStealing: true
    onCopyrightLinkActivated: Qt.openUrlExternally(link)
}
