/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kosmindoormap 1.0

Kirigami.OverlaySheet {
    id: gateSheet
    property var model

    header: Kirigami.Heading {
        text: i18n("Find Gate")
    }

    ListView {
        model: gateSheet.model

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
                gateSheet.sheetOpen = false
            }
        }
    }
}
