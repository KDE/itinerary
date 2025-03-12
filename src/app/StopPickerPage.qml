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

    Kirigami.PromptDialog {
        id: clearConfirmDialog
        title: i18n("Clear History")
        subtitle: i18n("Do you really want to remove all previously searched locations?")
        standardButtons: QQC2.Dialog.Cancel
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Remove")
                icon.name: "edit-clear-history"
                onTriggered: {
                    locationHistoryModel.clear();
                    deleteConfirmDialog.close();
                }
            }
        ]
    }

    QQC2.ActionGroup { id: sortActionGroup }
    actions: [
        Kirigami.Action {
            text: i18n("Clear history")
            icon.name: "edit-clear-history"
            onTriggered: clearConfirmDialog.open()
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

    function updateQuery()
    {
        if (queryTextField.text !== "" && countryCombo.currentValue !== "") {
            locationQueryModel.request = {
                location: {
                    name: queryTextField.text,
                    country: countryCombo.currentValue
                },
                types: Location.Stop | Location.Address
            };
        }
    }

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
                return sort([...new Set(countries)]);
            }
            initialCountry: root.initialCountry
            onCurrentValueChanged: root.updateQuery();
        }
        Kirigami.SearchField {
            id: queryTextField
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            onAccepted: root.updateQuery();
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
    TripGroupLocationModel {
        id: tripGroupLocationModel
        tripGroupManager: TripGroupManager
        tripGroupId: ApplicationController.contextTripGroupId
        onLocationsChanged: {
            locationHistoryModel.clearPresetLocations();
            for (let i = 0; i < tripGroupLocationModel.rowCount(); ++i) {
                const idx = tripGroupLocationModel.index(i, 0);
                locationHistoryModel.addPresetLocation(
                    tripGroupLocationModel.data(idx, TripGroupLocationModel.LocationRole),
                    tripGroupLocationModel.data(idx, TripGroupLocationModel.LastUsedRole),
                    tripGroupLocationModel.data(idx, TripGroupLocationModel.UseCountRole)
                );
            }
        }
    }
    KSortFilterProxyModel {
        id: historySortModel
        sourceModel: locationHistoryModel
        sortRoleName: Settings.read("StopPicker/historySortMode", "lastUsed")
        onSortRoleChanged: Settings.write("StopPicker/historySortMode", sortRoleName)
        sortOrder: sortRoleName == "locationName" ? Qt.AscendingOrder : Qt.DescendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
    }

    Component {
        id: historyDelegate
        Kirigami.SwipeListItem {
            id: delegate
            readonly property var sourceModel: ListView.view.model
            text: {
                let country = Country.fromAlpha2(model.location.country)
                let region = CountrySubdivision.fromCode(model.location.region)

                if (model.location.locality && model.location.name !== model.location.locality && region && country) {
                    return i18nc("location name, locality, region, country", "%1, %2, %3, %4",
                                 model.location.name,
                                 model.location.locality,
                                 region.name,
                                 country.name)
                } else if (model.location.locality && model.location.name !== model.location.locality && country) {
                    return i18nc("location name, locality, country", "%1, %2, %3",
                                 model.location.name,
                                 model.location.locality,
                                 country.name)
                } else if (region && country) {
                    return i18nc("location name, region, country", "%1, %2, %3",
                                 model.location.name,
                                 region.name,
                                 country.name)
                } else {
                    return model.location.name
                }
            }

            contentItem: Kirigami.IconTitleSubtitle {
                Accessible.ignored: true

                icon.name: model.location.iconName

                title: model.location.name

                subtitle: {
                    let country = Country.fromAlpha2(model.location.country)
                    let region = CountrySubdivision.fromCode(model.location.region)

                    if (model.location.locality && model.location.name !== model.location.locality && region && country) {
                        return i18nc("locality, region, country", "%1, %2, %3",
                                     model.location.locality,
                                     region.name,
                                     country.name)
                    } else if (model.location.locality && model.location.name !== model.location.locality && country) {
                        return i18nc("locality, country", "%1, %2",
                                     model.location.locality,
                                     country.name)
                    } else if (region && country) {
                        return i18nc("region, country", "%1, %2",
                                     region.name,
                                     country.name)
                    } else if (country) {
                        return country.name
                    } else {
                        return " "
                    }
                }
            }
            icon.name: model.location.iconName

            actions: [
                Kirigami.Action {
                    icon.name: "edit-delete"
                    text: i18n("Remove history entry")
                    onTriggered: {
                        sourceModel.removeRows(model.index, 1)
                    }
                    enabled: model.removable
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
            text: {
                let country = Country.fromAlpha2(model.location.country)
                let region = CountrySubdivision.fromCode(model.location.region)

                if (model.location.locality && model.location.name !== model.location.locality && region && country) {
                    return i18nc("location name, locality, region, country", "%1, %2, %3, %4",
                                 model.location.name,
                                 model.location.locality,
                                 region.name,
                                 country.name)
                } else if (model.location.locality && model.location.name !== model.location.locality && country) {
                    return i18nc("location name, locality, country", "%1, %2, %3",
                                 model.location.name,
                                 model.location.locality,
                                 country.name)
                } else if (region && country) {
                    return i18nc("location name, region, country", "%1, %2, %3",
                                 model.location.name,
                                 region.name,
                                 country.name)
                } else {
                    return model.location.name
                }
            }

            width: ListView.view.width
            contentItem: Kirigami.IconTitleSubtitle {
                Accessible.ignored: true

                icon.name: model.location.iconName

                title: model.location.name

                subtitle: {
                    let country = Country.fromAlpha2(model.location.country)
                    let region = CountrySubdivision.fromCode(model.location.region)

                    if (model.location.locality && model.location.name !== model.location.locality && region && country) {
                        return i18nc("locality, region, country", "%1, %2, %3",
                                     model.location.locality,
                                     region.name,
                                     country.name)
                    } else if (model.location.locality && model.location.name !== model.location.locality && country) {
                        return i18nc("locality, country", "%1, %2",
                                     model.location.locality,
                                     country.name)
                    } else if (region && country) {
                        return i18nc("region, country", "%1, %2",
                                     region.name,
                                     country.name)
                    } else if (country) {
                        return country.name
                    } else {
                        return " "
                    }
                }
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
