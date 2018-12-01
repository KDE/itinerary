/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

GridLayout {
    id: root
    property variant place

    function save(place) {
        var addr = place.address;
        addr.streetAddress = streetAddress.text;
        addr.postalCode = postalCode.text;
        addr.addressLocality = addressLocality.text;
        addr.addressRegion = addressRegion.text;
        addr.addressCountry = countryModel.isoCodeFromIndex(addressCountry.currentIndex)
        var newPlace = place;
        newPlace.address = addr;
        return newPlace;
    }

    GridLayout {
        columns: 2

        QQC2.Label {
            text: i18n("Street:")
        }
        QQC2.TextField {
            id: streetAddress
            text: place.address.streetAddress
        }

        QQC2.Label {
            text: i18n("Postal Code:")
        }
        QQC2.TextField {
            id: postalCode
            text: place.address.postalCode
        }

        QQC2.Label {
            text: i18n("City:")
        }
        QQC2.TextField {
            id: addressLocality
            text: place.address.addressLocality
        }

        QQC2.Label {
            text: i18n("Region:")
        }
        QQC2.TextField {
            id: addressRegion
            text: place.address.addressRegion
        }

        QQC2.Label {
            text: i18n("Country:")
        }
        CountryModel {
            id: countryModel
        }
        QQC2.ComboBox {
            id: addressCountry
            model: countryModel
            textRole: "display"
            currentIndex: countryModel.isoCodeToIndex(place.address.addressCountry)
        }
    }
}
