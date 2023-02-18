/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtLocation 5.15 as QtLocation
import QtPositioning 5.15
import org.kde.kirigami 2.17 as Kirigami
import org.kde.i18n.localeData 1.0
import org.kde.contacts 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.FormLayout {
    id: root
    property variant place
    property alias name: name.text
    property string nameLabel: i18n("Name:")

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

    QQC2.TextField {
        id: name
        Kirigami.FormData.label: nameLabel
    }

    QQC2.TextField {
        id: streetAddress
        Kirigami.FormData.label: i18n("Street:")
        text: place.address.streetAddress
    }

    QQC2.TextField {
        id: postalCode
        Kirigami.FormData.label: ("Postal Code:")
        text: place.address.postalCode

        readonly property bool validFormat: text.match('^' + root.addressFormat.postalCodeRegularExpression + '$')

        Kirigami.Icon {
            source: postalCode.validFormat ? "dialog-ok" : "dialog-warning"
            isMask: true
            color: postalCode.validFormat ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.neutralTextColor
            visible: postalCode.text && root.addressFormat.postalCodeRegularExpression
            height: Kirigami.Units.iconSizes.small
            width: height
            anchors.right: parent.right
            anchors.rightMargin: Kirigami.Units.smallSpacing
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    QQC2.TextField {
        id: addressLocality
        Kirigami.FormData.label: i18n("City:")
        text: place.address.addressLocality
    }

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
    QQC2.ComboBox {
        id: addressRegion
        Kirigami.FormData.label: i18n("Region:")
        editable: true
        model: regionModel
        textRole: "display"
        valueRole: "code"
        Layout.fillWidth: true
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

    App.CountryComboBox {
        id: addressCountry
        Kirigami.FormData.label: i18n("Country:")
        model: Country.allCountries.map(c => c.alpha2)
        initialCountry: place.address.addressCountry ? place.address.addressCountry : root.defaultCountry
        Layout.fillWidth: true
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
                addressRegion.tryFindRegion(loc.state);
                // countryCode is supposed to be the code already, but isn't always...
                addressCountry.currentIndex = addressCountry.indexOfValue(Country.fromName(loc.countryCode).alpha2);
            }
        }
        onErrorStringChanged: showPassiveNotification(geocodeModel.errorString, "short")
    }

    RowLayout  {
        Kirigami.FormData.label: i18n("Coordinate:")
        QQC2.Button {
            icon.name: "crosshairs"
            onClicked: applicationWindow().pageStack.push(locationPickerPage);
            Accessible.name: i18n("Pick coordinate")
            QQC2.ToolTip.text: Accessible.name
            // TODO we can autofill country/region using KCountry[Subdivision] here?
        }
        QQC2.Label {
            visible: !Number.isNaN(root.latitude) && !Number.isNaN(root.longitude)
            text: i18n("%1°, %2°", root.latitude.toFixed(2), root.longitude.toFixed(2));
        }
        QQC2.Button {
            icon.name: "go-down-symbolic"
            enabled: geocodeModel.status !== QtLocation.GeocodeModel.Loading
            visible: addressLocality.text
            onClicked: {
                geocodeAddr.street = streetAddress.text;
                geocodeAddr.postalCode = postalCode.text;
                geocodeAddr.city = addressLocality.text;
                geocodeAddr.state = addressRegion.currentValue ? addressRegion.currentValue : "";
                geocodeAddr.countryCode = addressCountry.currentValue;
                geocodeModel.update();
            }
            Accessible.name: i18n("Find coordinate from address")
            QQC2.ToolTip.text: Accessible.name
        }
        QQC2.Button {
            icon.name: "go-up-symbolic"
            enabled: reverseGeocodeModel.status !== QtLocation.GeocodeModel.Loading
            visible: !Number.isNaN(root.latitude) && !Number.isNaN(root.longitude)
            onClicked: {
                reverseGeocodeModel.query = QtPositioning.coordinate(root.latitude, root.longitude)
                reverseGeocodeModel.update();
            }
            Accessible.name: i18n("Fill address from coordinate")
            QQC2.ToolTip.text: Accessible.name
        }
    }
}
