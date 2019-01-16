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

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QObject>

class QVariant;

/** Date/time localization utilities.
 *  Works around JS losing timezone information, ie. we need
 *  to do this without passing the date/time values through JS.
 */
class Localizer : public QObject
{
    Q_OBJECT
public:
    explicit Localizer(QObject *parent = nullptr);
    ~Localizer();

    Q_INVOKABLE QString countryName(const QString &isoCode) const;
    Q_INVOKABLE QString formatAddress(const QVariant &obj) const;
    Q_INVOKABLE QString formatTime(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDateTime(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDuration(int seconds) const;
};

#endif // LOCALIZER_H
