/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels
import org.kde.i18n.localeData
import org.kde.kpublictransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    property var publicTransportManager
    /**
     * Initially selected country.
     * If not specified the country from the current locale is used.
     */
    property string initialCountry: Qt.locale().name.match(/_([A-Z]{2})/)[1]
    property var location

    QQC2.ActionGroup { id: sortActionGroup }
    actions: [
        Kirigami.Action {
            text: i18n("Clear history")
            icon.name: "edit-clear-history"
            onTriggered: locationHistoryModel.clear()
        },
        Kirigami.Action { separator: true },

        Kirigami.Action {
            QQC2.ActionGroup.group: sortActionGroup
            checkable: true
            checked: historySortModel.sortRoleName == "locationName"
            text: i18n("Sort by name")
            onTriggered: historySortModel.sortRoleName = "locationName"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: sortActionGroup
            checkable: true
            checked: historySortModel.sortRoleName == "lastUsed"
            text: i18n("Most recently used")
            onTriggered: historySortModel.sortRoleName = "lastUsed"
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: sortActionGroup
            checkable: true
            checked: historySortModel.sortRoleName == "useCount"
            text: i18n("Most often used")
            onTriggered: historySortModel.sortRoleName = "useCount"
        }
    ]
    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        CountryComboBox {
            id: countryCombo
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            model: {
                var countries = new Array();
                for (const b of publicTransportManager.backends) {
                    if (!publicTransportManager.isBackendEnabled(b.identifier)) {
                        continue;
                    }
                    for (const t of [CoverageArea.Realtime, CoverageArea.Regular, CoverageArea.Any]) {
                        for (const c of b.coverageArea(t).regions) {
                            if (c != 'UN' && c != 'EU') {
                                countries.push(c.substr(0, 2));
                            }
                        }
                    }
                }
                return [...new Set(countries)];
            }
            initialCountry: root.initialCountry
            onCurrentValueChanged: {
                var loc = locationQueryModel.request.location;
                loc.country = countryCombo.currentValue;
                locationQueryModel.request.location = loc;
                locationQueryModel.request.type = Location.Stop
            }
        }
        Kirigami.SearchField {
            id: queryTextField
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            onAccepted: {
                if (text !== "" && countryCombo.currentValue !== "") {
                    var loc = locationQueryModel.request.location;
                    loc.name = text;
                    loc.country = countryCombo.currentValue;
                    locationQueryModel.request.location = loc;
                    locationQueryModel.request.type = Location.Stop
                }
            }
        }
    }

    LocationQueryModel {
        id: locationQueryModel
        manager: publicTransportManager
        queryDelay: 500
    }
    LocationHistoryModel {
        id: locationHistoryModel
    }
    KSortFilterProxyModel {
        id: historySortModel
        sourceModel: locationHistoryModel
        sortRoleName: Settings.read("StopPicker/historySortMode", "lastUsed")
        onSortRoleChanged: Settings.write("StopPicker/historySortMode", sortRoleName)
        sortOrder: sortRoleName == "locationName" ? Qt.AscendingOrder : Qt.DescendingOrder
    }

    Component {
        id: historyDelegate
        Kirigami.SwipeListItem {
            id: delegate
            readonly property var sourceModel: ListView.view.model
            text: model.location.name
            contentItem: QQC2.Label {
                text: model.location.name
                elide: Text.ElideRight
                Accessible.ignored: true
            }
            actions: [
                Kirigami.Action {
                    icon.name: "edit-delete"
                    text: i18n("Remove history entry")
                    onTriggered: {
                        sourceModel.removeRows(model.index, 1)
                    }
                }
            ]
            onClicked: {
                root.location = model.location;
                locationHistoryModel.addLocation(model.location);
                applicationWindow().pageStack.goBack();
            }
            Accessible.onPressAction: delegate.clicked()
        }
    }

    Component {
        id: queryResultDelegate
        QQC2.ItemDelegate {
            id: delegate
            text: model.location.name
            width: ListView.view.width
            contentItem: Kirigami.TitleSubtitle {
                title: model.location.name
            }
            onClicked: {
                root.location = model.location
                locationHistoryModel.addLocation(model.location);
                applicationWindow().pageStack.goBack();
                queryTextField.clear();
            }
            Accessible.onPressAction: delegate.clicked()
        }
    }

    ListView {
        id: locationView
        model: queryTextField.text === "" ? historySortModel : locationQueryModel
        delegate: queryTextField.text === "" ? historyDelegate : queryResultDelegate

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: locationQueryModel.loading
        }

        QQC2.Label {
            anchors.centerIn: parent
            width: parent.width
            text: locationQueryModel.errorMessage
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }

        Kirigami.PlaceholderMessage {
            text: i18n("No locations found")
            visible: locationView.count === 0 && !locationQueryModel.loading && queryTextField !== ""
            anchors.centerIn: parent
        }
    }
}
