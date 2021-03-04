/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

RowLayout {
    property var journey

    Repeater {
        model: journey.sections
        delegate: Kirigami.Icon {
            source: {
                switch (modelData.mode) {
                    case JourneySection.PublicTransport:
                        return PublicTransport.lineIcon(modelData.route.line);
                    case JourneySection.Walking: return "qrc:///images/walk.svg";
                    case JourneySection.Waiting: return "qrc:///images/wait.svg";
                    case JourneySection.Transfer: return "qrc:///images/transfer.svg";
                    case JourneySection.RentedVehicle:
                        return PublicTransport.rentalVehicleIcon(modelData.rentalVehicle);
                    default: return "question";
                }
            }
            color: PublicTransport.warnAboutSection(modelData) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            width: isMask ? height : implicitWidth
            height: Kirigami.Units.iconSizes.small
            isMask: modelData.mode != JourneySection.PublicTransport || (!modelData.route.line.hasLogo && !modelData.route.line.hasModeLogo)
            Layout.preferredWidth: width
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
        }
    }
    QQC2.Label {
        text: i18np("One change", "%1 changes", journey.numberOfChanges)
    }
}
