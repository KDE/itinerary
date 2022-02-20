/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtMultimedia 5.15
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison.scanner 1.0 as Prison
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    title: i18n("Scan Barcode")

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
        source: camera
        filters: [scanner]
        autoOrientation: true
        fillMode: VideoOutput.PreserveAspectCrop
    }

    Prison.VideoScanner {
        id: scanner
        formats: Prison.Format.QRCode | Prison.Format.Aztec | Prison.Format.DataMatrix | Prison.Format.PDF417
        onResultContentChanged: {
            if (result.hasText && ApplicationController.importText(result.text)) {
                applicationWindow().pageStack.goBack();
            }
            if (result.hasBinaryData && ApplicationController.importData(result.binaryData)) {
                applicationWindow().pageStack.goBack();
            }
        }
    }

    Camera {
        id: camera
        focus {
            focusMode: Camera.FocusContinuous
            focusPointMode: Camera.FocusPointCenter
        }
    }

    Rectangle {
        border {
            color: Kirigami.Theme.focusColor
            width: 2
        }
        color: Qt.rgba(Kirigami.Theme.focusColor.r, Kirigami.Theme.focusColor.g, Kirigami.Theme.focusColor.b, 0.2);
        radius: Kirigami.Units.smallSpacing

        x: viewFinder.mapRectToItem(scanner.result.boundingRect).x
        y: viewFinder.mapRectToItem(scanner.result.boundingRect).y
        width: viewFinder.mapRectToItem(scanner.result.boundingRect).width
        height: viewFinder.mapRectToItem(scanner.result.boundingRect).height
    }

    BarcodeScanModeController {
        page: root
        fullBrightness: false
        enabled: camera.errorCode != Camera.NoError
    }

    Kirigami.PlaceholderMessage {
        anchors.fill: parent
        text: i18n("No camera available.")
        visible: camera.errorCode != Camera.NoError
    }
}
