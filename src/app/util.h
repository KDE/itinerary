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

#ifndef UTIL_H
#define UTIL_H

#include <QObject>

class QDateTime;

/** Misc utilities. */
class Util : public QObject
{
    Q_OBJECT
public:
    explicit Util(QObject *parent = nullptr);
    ~Util();

    // workarounds for JS not preserving timezones

    /** Read a QDateTime property with the timezone stripped off. */
    Q_INVOKABLE QDateTime dateTimeStripTimezone(const QVariant &obj, const QString &propertyName) const;
    /** Set a QDateTime property preserving the timezone of the current value. */
    Q_INVOKABLE QVariant setDateTimePreserveTimezone(const QVariant &obj, const QString &propertyName, QDateTime value) const;

    /** Convert links, email address and phone numbers in the given text to HTML links. */
    Q_INVOKABLE QString textToHtml(const QString &text) const;

    /** Execute the non-exported sort() method on a QAbstractItemModel. */
    Q_INVOKABLE void sortModel(QObject *model, int column, Qt::SortOrder sortOrder) const;
};

#endif // UTIL_H
