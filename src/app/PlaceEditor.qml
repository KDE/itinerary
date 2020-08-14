/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtPositioning 5.11
import org.kde.kirigami 2.5 as Kirigami
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

    function save(place) {
        var addr = place.address;
        addr.streetAddress = streetAddress.text;
        addr.postalCode = postalCode.text;
        addr.addressLocality = addressLocality.text;
        addr.addressRegion = addressRegion.text;
        addr.addressCountry = countryModel.isoCodeFromIndex(addressCountry.currentIndex)
        var geo = place.geo;
        geo.latitude = latitude;
        geo.longitude = longitude;
        var newPlace = place;
        newPlace.address = addr;
        newPlace.geo = geo;
        return newPlace;
    }

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
    }

    QQC2.TextField {
        id: addressLocality
        Kirigami.FormData.label: i18n("City:")
        text: place.address.addressLocality
    }

    QQC2.TextField {
        id: addressRegion
        Kirigami.FormData.label: i18n("Region:")
        text: place.address.addressRegion
    }

    CountryModel {
        id: countryModel
    }
    QQC2.ComboBox {
        id: addressCountry
        Kirigami.FormData.label: i18n("Country:")
        model: countryModel
        textRole: "display"
        currentIndex: countryModel.isoCodeToIndex(place.address.addressCountry)
    }

    RowLayout  {
        QQC2.Button {
            icon.name: "crosshairs"
            onClicked: applicationWindow().pageStack.push(locationPickerPage);
        }
        QQC2.Label {
            visible: !Number.isNaN(root.latitude) && !Number.isNaN(root.longitude)
            text: i18n("%1°, %2°", root.latitude.toFixed(2), root.longitude.toFixed(2));
        }
    }
}
