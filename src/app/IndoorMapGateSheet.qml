/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

Kirigami.OverlaySheet {
    id: gateSheet
    property var model

    header: Kirigami.Heading {
        text: i18n("Find Gate")
    }

    ListView {
        model: gateSheet.model
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10

        delegate: Kirigami.BasicListItem {
            property var gate: model
            text: {
                if (gate.isDepartureGate && gate.isArrivalGate)
                    return i18nc("flight departure/arrival gate", "%1 (arrival + departure)", gate.display);
                if (gate.isDepartureGate)
                    return i18nc("flight departure gate", "%1 (departure)", gate.display);
                if (gate.isArrivalGate)
                    return i18nc("flight arrival gate", "%1 (arrival)", gate.display);
                return gate.display
            }
            highlighted: false
            onClicked: {
                map.view.floorLevel = model.level
                map.view.centerOnGeoCoordinate(model.coordinate);
                map.view.setZoomLevel(18, Qt.point(map.width / 2.0, map.height/ 2.0));
                gateSheet.close();
            }
        }
    }
}
