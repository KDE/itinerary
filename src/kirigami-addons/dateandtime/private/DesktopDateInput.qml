/*
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary.kirigamiaddons.dateandtime 0.1

RowLayout {
    id: layout

    //DAVE - if I'm in an RTL country are my date formats pre-reversed?
    //i.e Wikipedia says afghan is d/m/yyyy   should year go on the left or the right?

    property date value
    property alias selectedDate: layout.value

    property string dateFormat: Qt.locale().dateFormat(Locale.ShortFormat)


    //date formats can be in big endian (china), little endian (Europe), or absolutely ridiculous endian (US)
    //separators are also different
    Component.onCompleted: {
        for (var i in layout.children) {
            layout.children[i].destroy();
        }

        var parse = /([^dMy]*)([dMy]+)([^dMy]*)([dMy]+)([^dMy]*)([dMy]+)([^dMy]*)/
        var parts = parse.exec(dateFormat);
        for(var i=1; i < parts.length; i++) {
            var part = parts[i];

            if (!part) {
                continue;
            }

            if (part.startsWith("d")) {
                daySelectComponent.createObject(layout);
            } else if (part.startsWith("M")) {
                monthSelectComponent.createObject(layout);
            } else if (part.startsWith("y")) {
                yearSelectComponent.createObject(layout);
            }
        }
    }

    Component {
        id: daySelectComponent

        SpinBox {
            function daysInMonth (year, month) {
                // we want the days in @p month but go back one day from its start, therefore +1
                return new Date(year, month + 1, 0).getDate();
            }

            from: 1
            to: {
                return daysInMonth(layout.value.getFullYear(),
                                   layout.value.getMonth());
            }
            editable: true
            value: layout.value.getDate();
            onValueModified: {
                var dt = layout.value;
                dt.setDate(value);
                layout.value = dt;
            }
        }
    }

    Component {
        id: monthSelectComponent
        ComboBox {
            id: combo
            // JS Date months start at 0, so we can map directly
            currentIndex: layout.value.getMonth() // DAVE should be a binding
            textRole: "display"
            model: YearModel {
                year: layout.value.getFullYear()
            }
            onActivated: {
                var dt = layout.value;
                dt.setMonth(currentIndex);
                layout.value = dt;
            }
        }
    }

    Component {
        id: yearSelectComponent
        SpinBox {
            from: 1970
            to: 2100 //I assume we'll have a new LTS release by then
            editable: true
            //default implementation does toLocaleString which looks super weird adding a comma
            textFromValue: function(value) {return value}
            value: layout.value.getFullYear();
            onValueModified: {
                var dt = layout.value;
                dt.setFullYear(value);
                layout.value = dt;
            }
        }
    }
}
