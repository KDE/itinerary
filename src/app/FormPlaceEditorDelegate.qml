/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.i18n.localeData
import org.kde.contacts
import org.kde.kitinerary
import org.kde.itinerary

ColumnLayout {
    id: root
    property variant place

    property double latitude: place.geo.latitude;
    property double longitude: place.geo.longitude;

    /** Currently selected country code. */
    readonly property string currentCountry: addressCountry.currentValue

    function save(place) {
        var addr = place.address;
        addr.streetAddress = streetAddress.text;
        addr.postalCode = postalCode.text;
        addr.addressLocality = addressLocality.text;
        if (addressRegion.visible) {
            addr.addressRegion = addressRegion.currentIndex < 0 ? addressRegion.editText : addressRegion.currentValue.substr(3);
        } else {
            addr.addressRegion = '';
        }
        addr.addressCountry = addressCountry.currentValue;
        var geo = place.geo;
        geo.latitude = latitude;
        geo.longitude = longitude;
        var newPlace = place;
        newPlace.address = addr;
        newPlace.geo = geo;
        return newPlace;
    }

    readonly property var addressFormat: AddressFormatRepository.formatForCountry(addressCountry.currentValue, KContacts.AddressFormatScriptPreference.Local)

    spacing: 0

    Component {
        id: locationPickerPage
        LocationPicker {
            title: i18n("Pick Location")
            coordinate: QtPositioning.coordinate(root.latitude, root.longitude)
            onCoordinateChanged: {
                root.latitude = coordinate.latitude;
                root.longitude = coordinate.longitude;
            }
        }
    }

    FormCard.FormTextFieldDelegate {
        id: streetAddress
        label: i18n("Street")
        text: place.address.streetAddress
    }
    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: postalCode
        label: ("Postal Code")
        text: place.address.postalCode

        readonly property bool validFormat: root.addressFormat.postalCodeRegularExpression === '' || text.match('^' + root.addressFormat.postalCodeRegularExpression + '$')
        status: validFormat ? Kirigami.MessageType.Positive : Kirigami.MessageType.Warning
        statusMessage: postalCode.text && !validFormat ? i18n("Invalid postal code format for this country.") : "";
    }
    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: addressLocality
        label: i18n("City")
        text: place.address.addressLocality
    }
    FormCard.FormDelegateSeparator {}

    CountrySubdivisionModel {
        id: regionModel
        country: addressCountry.currentCountry
        onCountryChanged: {
            if (addressRegion.currentIndex < 0) {
                addressRegion.tryFindRegion(addressRegion.editText != '' ? addressRegion.editText : place.address.addressRegion);
            } else {
                addressRegion.currentIndex = -1;
            }
        }
    }
    FormCard.FormComboBoxDelegate {
        id: addressRegion
        text: i18n("Region")
        editable: true
        model: regionModel
        textRole: "display"
        valueRole: "code"
        visible: (root.addressFormat.usedFields & KContacts.AddressFormatField.Region) || editText

        function tryFindRegion(input) {
            const i = regionModel.rowForNameOrCode(input)
            if (i >= 0) {
                currentIndex = i;
            } else if (input) {
                editText = input;
            } else {
                currentIndex = -1;
                editText = '';
            }
        }

        Component.onCompleted: tryFindRegion(place.address.addressRegion)
        onAccepted: tryFindRegion(editText)
    }
    FormCard.FormDelegateSeparator { visible: addressRegion.visible }

    CountryComboBoxDelegate {
        id: addressCountry
        text: i18n("Country")
        model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
            return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
        })
        initialCountry: place.address.addressCountry
    }

    QtLocation.GeocodeModel {
        id: geocodeModel
        plugin: applicationWindow().osmPlugin()
        autoUpdate: false
        limit: 1

        query: Address {
            id: geocodeAddr
        }

        onLocationsChanged: {
            let locs = [];
            for (let i = 0; i < count; ++i)
                locs.push(geocodeModel.get(i));

            // filter wrong matches that we might get here
            // apparently the Nominatim result order is lost along the way, so we can't rely
            // the first entry being the best one
            if (postalCode.text !== "") {
                locs = locs.filter(l => l.address.postalCode == postalCode.text);
            }
            if (locs.length > 0) {
                root.latitude = locs[0].coordinate.latitude;
                root.longitude = locs[0].coordinate.longitude;
                return;
            }

            if (count >= 1) {
                root.latitude = geocodeModel.get(0).coordinate.latitude;
                root.longitude = geocodeModel.get(0).coordinate.longitude;
            }
        }
        onErrorStringChanged: showPassiveNotification(geocodeModel.errorString, "short")
    }
    QtLocation.GeocodeModel {
        id: reverseGeocodeModel
        plugin: applicationWindow().osmPlugin()
        autoUpdate: false
        limit: 1
        onLocationsChanged: {
            if (count >= 1) {
                const loc = reverseGeocodeModel.get(0).address;
                streetAddress.text = loc.street;
                postalCode.text = loc.postalCode
                addressLocality.text = loc.city;
                // countryCode is supposed to be the code already, but isn't always...
                addressCountry.currentIndex = addressCountry.indexOfValue(Country.fromName(loc.countryCode).alpha2);
                // only apply region if the new country actually needs that
                if (root.addressFormat.usedFields & KContacts.AddressFormatField.Region) {
                    addressRegion.tryFindRegion(loc.state);
                } else {
                    addressRegion.currentIndex = -1;
                    addressRegion.editText = '';
                }
            }
        }
        onErrorStringChanged: showPassiveNotification(geocodeModel.errorString, "short")
    }

    FormCard.FormButtonDelegate {
        text: i18n("Coordinate")
        description: !Number.isNaN(root.latitude) && !Number.isNaN(root.longitude) ? i18n("%1°, %2°", root.latitude.toFixed(2), root.longitude.toFixed(2)) : i18n("Pick…");
        icon.name: "crosshairs"
        onClicked: applicationWindow().pageStack.push(locationPickerPage)
        // TODO we can autofill country/region using KCountry[Subdivision] here?
    }

    FormCard.FormDelegateSeparator {
        visible: addressToCoordinateAction.visible || coordinateToAddressAction.visible
    }

    FormCard.AbstractFormDelegate {
        visible: addressToCoordinateAction.visible || coordinateToAddressAction.visible

        contentItem: Kirigami.ActionToolBar {
            actions: [
                Kirigami.Action {
                    id: addressToCoordinateAction

                    text: i18n("Address to coordinate")
                    icon.name: "go-down-symbolic"
                    enabled: geocodeModel.status !== QtLocation.GeocodeModel.Loading
                    visible: addressLocality.text
                    onTriggered: {
                        geocodeAddr.street = streetAddress.text;
                        geocodeAddr.postalCode = postalCode.text;
                        geocodeAddr.city = addressLocality.text;
                        geocodeAddr.state = addressRegion.currentValue ? addressRegion.currentValue : "";
                        geocodeAddr.countryCode = addressCountry.currentValue;
                        geocodeModel.update();
                    }
                },
                Kirigami.Action {
                    id: coordinateToAddressAction

                    text: i18n("Coordinate to address")
                    icon.name: "go-up-symbolic"
                    enabled: reverseGeocodeModel.status !== QtLocation.GeocodeModel.Loading
                    visible: !Number.isNaN(root.latitude) && !Number.isNaN(root.longitude)
                    onTriggered: {
                        reverseGeocodeModel.query = QtPositioning.coordinate(root.latitude, root.longitude)
                        reverseGeocodeModel.update();
                    }
                }
            ]
        }
    }
}
