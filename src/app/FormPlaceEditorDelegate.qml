/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtLocation @QTLOCATION_MODULE_VERSION@ as QtLocation
import QtPositioning 5.15
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.i18n.localeData 1.0
import org.kde.contacts 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

ColumnLayout {
    id: root
    property variant place

    property double latitude: place.geo.latitude;
    property double longitude: place.geo.longitude;

    /** Initially selected country if the given address doesn't specify one. */
    property string defaultCountry: Settings.homeCountryIsoCode

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

    MobileForm.FormTextFieldDelegate {
        id: streetAddress
        label: i18n("Street")
        text: place.address.streetAddress
    }
    MobileForm.FormDelegateSeparator {}

    MobileForm.FormTextFieldDelegate {
        id: postalCode
        label: ("Postal Code")
        text: place.address.postalCode

        readonly property bool validFormat: root.addressFormat.postalCodeRegularExpression === '' || text.match('^' + root.addressFormat.postalCodeRegularExpression + '$')
        status: validFormat ? Kirigami.MessageType.Positive : Kirigami.MessageType.Warning
        statusMessage: postalCode.text && !validFormat ? i18n("Invalid postal code format for this country.") : "";
    }
    MobileForm.FormDelegateSeparator {}

    MobileForm.FormTextFieldDelegate {
        id: addressLocality
        label: i18n("City")
        text: place.address.addressLocality
    }
    MobileForm.FormDelegateSeparator {}

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
    MobileForm.FormComboBoxDelegate {
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
    MobileForm.FormDelegateSeparator { visible: addressRegion.visible }

    App.CountryComboBoxDelegate {
        id: addressCountry
        text: i18n("Country")
        model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
            return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
        })
        initialCountry: place.address.addressCountry ? place.address.addressCountry : root.defaultCountry
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

    MobileForm.FormButtonDelegate {
        text: i18n("Coordinate")
        description: !Number.isNaN(root.latitude) && !Number.isNaN(root.longitude) ? i18n("%1°, %2°", root.latitude.toFixed(2), root.longitude.toFixed(2)) : i18n("Pick...");
        icon.name: "crosshairs"
        onClicked: applicationWindow().pageStack.push(locationPickerPage)
        // TODO we can autofill country/region using KCountry[Subdivision] here?
    }

    MobileForm.AbstractFormDelegate {
        contentItem: Kirigami.ActionToolBar {
            actions: [
                Kirigami.Action {
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
