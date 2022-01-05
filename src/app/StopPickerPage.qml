/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.i18n.localeData 1.0
import org.kde.kpublictransport 1.0
import "." as App

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
    actions.contextualActions: [
        Kirigami.Action {
            text: i18n("Clear history")
            iconName: "edit-clear-history"
            onTriggered: locationHistoryModel.clear()
        },
        Kirigami.Action { separator: true },

        // TODO store sort mode
        Kirigami.Action {
            QQC2.ActionGroup.group: sortActionGroup
            checkable: true
            text: i18n("Sort by name")
            onTriggered: {
                historySortModel.sortRole = "locationName";
                historySortModel.sortOrder = Qt.AscendingOrder;
            }
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: sortActionGroup
            checkable: true
            checked: true
            text: i18n("Most recently used")
            onTriggered: {
                historySortModel.sortRole = "lastUsed";
                historySortModel.sortOrder = Qt.DescendingOrder;
            }
        },
        Kirigami.Action {
            QQC2.ActionGroup.group: sortActionGroup
            checkable: true
            text: i18n("Most often used")
            onTriggered: {
                historySortModel.sortRole = "useCount";
                historySortModel.sortOrder = Qt.DescendingOrder;
            }
        }
    ]
    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        App.CountryComboBox {
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
        sortRole: "lastUsed"
        sortOrder: Qt.DescendingOrder
    }

    Component {
        id: historyDelegate
        Kirigami.SwipeListItem {
            readonly property var sourceModel: ListView.view.model
            QQC2.Label {
                text: model.location.name
            }
            actions: [
                Kirigami.Action {
                    iconName: "edit-delete"
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
        }
    }

    Component {
        id: queryResultDelegate
        Kirigami.BasicListItem {
            text: model.location.name
            onClicked: {
                root.location = model.location
                locationHistoryModel.addLocation(model.location);
                applicationWindow().pageStack.goBack();
                queryTextField.clear();
            }
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
