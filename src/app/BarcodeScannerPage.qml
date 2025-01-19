/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtMultimedia
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.prison.scanner as Prison
import org.kde.itinerary

Kirigami.Page {
    id: root
    title: i18n("Scan Barcode")

    property alias supportedFormats: scanner.formats

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    signal barcodeDetected(var result)

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
    }

    Prison.VideoScanner {
        id: scanner
        formats: Prison.Format.QRCode | Prison.Format.Aztec | Prison.Format.DataMatrix | Prison.Format.PDF417
        videoSink: viewFinder.videoSink
        onResultContentChanged: root.barcodeDetected(result)
    }

    CaptureSession {
        id: captureSession
        camera: Camera {
            id: camera
            active: true
        }
        videoOutput: viewFinder
    }

    Rectangle {
        border {
            color: Kirigami.Theme.focusColor
            width: 2
        }
        color: Qt.rgba(Kirigami.Theme.focusColor.r, Kirigami.Theme.focusColor.g, Kirigami.Theme.focusColor.b, 0.2);
        radius: Kirigami.Units.smallSpacing

        x: viewFinder.contentRect.x + scanner.result.boundingRect.x / viewFinder.sourceRect.width * viewFinder.contentRect.width
        y: viewFinder.contentRect.y + scanner.result.boundingRect.y / viewFinder.sourceRect.height * viewFinder.contentRect.height
        width: scanner.result.boundingRect.width / viewFinder.sourceRect.width * viewFinder.contentRect.width
        height: scanner.result.boundingRect.height / viewFinder.sourceRect.height * viewFinder.contentRect.height
    }

    BarcodeScanModeController {
        page: root
        fullBrightness: false
        enabled: camera.error != Camera.NoError
    }

    Kirigami.PlaceholderMessage {
        anchors.fill: parent
        text: i18n("No camera available.")
        visible: camera.error != Camera.NoError
    }

    FloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: Kirigami.Action {
            icon.name: checked ? "flashlight-off" : "flashlight-on"
            text: i18n("Light")
            checkable: true
            checked: camera.torchMode == Camera.TorchOn
            visible: camera.isTorchModeSupported
            enabled: camera.error == Camera.NoError
            onTriggered: camera.torchMode = (camera.torchMode == Camera.TorchOn ? Camera.TorchOff : Camera.TorchOn)
        }
    }

    Component.onCompleted: PermissionManager.requestPermission(Permission.Camera, function() { camera.start(); })
}
