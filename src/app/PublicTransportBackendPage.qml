/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.10 as Kirigami
import org.kde.kpublictransport 1.0 as KPublicTransport
import org.kde.itinerary 1.0

Kirigami.ScrollablePage {
    id: root
    title: i18n("Public Transport Information Sources")

    property alias publicTransportManager: backendModel.manager

    KPublicTransport.BackendModel {
        id: backendModel
    }

    Component {
        id: backendDelegate
        Kirigami.AbstractListItem {
            highlighted: false
            enabled: model.itemEnabled

            Item {
                anchors.margins: Kirigami.Units.largeSpacing
                implicitHeight: childrenRect.height

                QQC2.Label {
                    id: nameLabel
                    text: model.name
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.right: securityIcon.left
                    anchors.rightMargin: Kirigami.Units.largeSpacing
                    // try to retain trailing abbreviations when we have to elide
                    elide: text.endsWith(")") ? Text.ElideMiddle : Text.ElideRight
                }
                Kirigami.Icon {
                    id: securityIcon
                    source: model.isSecure ? "channel-secure-symbolic" : "channel-insecure-symbolic"
                    color: model.isSecure ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                    width: height
                    height: Kirigami.Units.gridUnit
                    anchors.top: parent.top
                    anchors.right: toggle.left
                }
                QQC2.Switch {
                    id: toggle
                    checked: model.backendEnabled
                    onToggled: model.backendEnabled = checked;
                    anchors.top: parent.top
                    anchors.right: parent.right
                }
                QQC2.Label {
                    anchors.top: nameLabel.bottom
                    anchors.left: parent.left
                    anchors.right: toggle.left
                    anchors.topMargin: Kirigami.Units.smallSpacing
                    text: model.description
                    font.italic: true
                    wrapMode: Text.WordWrap
                }
            }

            onClicked: {
                toggle.toggle(); // does not trigger the signal handler for toggled...
                model.backendEnabled = toggle.checked;
            }
        }
    }

    ListView {
        model: backendModel
        delegate: backendDelegate

        section.property: "primaryCountryCode"
        section.delegate: Kirigami.ListSectionHeader {
            text: section == "" ? i18n("Global") : Localizer.countryFlag(section) + " " + Localizer.countryName(section)
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
    }
}
