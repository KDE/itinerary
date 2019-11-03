/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kpublictransport 1.0 as KPublicTransport

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

            GridLayout {
                columns: 3
                rows: 2

                QQC2.Label {
                    text: model.name
                    Layout.fillWidth: true
                }
                Kirigami.Icon {
                    source: model.isSecure ? "channel-secure-symbolic" : "channel-insecure-symbolic"
                    color: model.isSecure ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                    width: height
                    height: Kirigami.Units.gridUnit
                }
                QQC2.Switch {
                    id: toggle
                    checked: model.backendEnabled
                    Layout.rowSpan: 2
                    onToggled: model.backendEnabled = checked;
                }
                QQC2.Label {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: model.description
                    font.italic: true
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
    }
}
