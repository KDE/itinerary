/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as PublicTransport
import org.kde.kpublictransport.ui as PublicTransport
import org.kde.kosmindoormap as KOSM

Kirigami.Dialog {
    id: root

    property KOSM.PlatformModel model

    signal platformSelected(platform: KOSM.platform)

    title: i18nc("@title", "Find Platform")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        model: root.model
        clip: true

        delegate: QQC2.ItemDelegate {
            id: delegateRoot
            required property KOSM.platform platform
            required property bool isDeparturePlatform
            required property bool isArrivalPlatform

            width: ListView.view.width
            contentItem: Row {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    id: label
                    text: {
                        if (delegateRoot.isDeparturePlatform && delegateRoot.isArrivalPlatform)
                            return i18nc("train arrival/departure platform", "%1 (arrival/departure)", delegateRoot.platform.name);
                        if (delegateRoot.isDeparturePlatform)
                            return i18nc("train departure platform", "%1 (departure)", delegateRoot.platform.name);
                        if (delegateRoot.isArrivalPlatform)
                            return i18nc("train arrival platform", "%1 (arrival)", delegateRoot.platform.name);
                        return delegateRoot.platform.name
                    }
                }

                Repeater {
                    model: delegateRoot.platform.lines
                    delegate: PublicTransport.TransportIcon {
                        required property string modelData
                        iconHeight: Kirigami.Units.iconSizes.small
                        anchors.verticalCenter: label.verticalCenter
                        visible: source != ""
                        source: {
                            switch (delegateRoot.platform.mode) {
                                case KOSM.Platform.Rail:
                                    return PublicTransport.LineMetaData.lookup(modelData, delegateRoot.platform.position.y, delegateRoot.platform.position.x, PublicTransport.Line.Train, true).logo;
                                case KOSM.Platform.Tram:
                                    return PublicTransport.LineMetaData.lookup(modelData, delegateRoot.platform.position.y, delegateRoot.platform.position.x, PublicTransport.Line.Tramway, true).logo;
                                case KOSM.Platform.Subway:
                                    return PublicTransport.LineMetaData.lookup(modelData, delegateRoot.platform.position.y, delegateRoot.platform.position.x, PublicTransport.Line.Metro, true).logo;
                            }
                            return "";
                        }
                    }
                }
            }
            highlighted: false
            onClicked: {
                root.platformSelected(delegateRoot.platform);
                root.close();
            }
        }

        section.property: "mode"
        section.delegate: Kirigami.ListSectionHeader {
            text: KOSM.PlatformUtil.modeName(parseInt(section))
            width: ListView.view.width
        }
        section.criteria: ViewSection.FullString
    }
}
