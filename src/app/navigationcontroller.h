/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#ifndef NAVIGATIONCONTROLLER_H
#define NAVIGATIONCONTROLLER_H

#include <QMetaType>

class QGeoPositionInfo;
class QGeoPositionInfoSource;
class QVariant;

/** Navigation and map display API for use from QML. */
class NavigationController
{
    Q_GADGET
public:
    Q_INVOKABLE void showOnMap(const QVariant &place);

    Q_INVOKABLE bool canNavigateTo(const QVariant &place);
    Q_INVOKABLE void navigateTo(const QVariant &place);
    Q_INVOKABLE void navigateTo(const QVariant &from, const QVariant &to);

private:
#ifndef Q_OS_ANDROID
    void navigateTo(const QGeoPositionInfo &from, const QVariant &to);

    QGeoPositionInfoSource *m_positionSource = nullptr;
    QMetaObject::Connection m_pendingNavigation;
#endif
};

Q_DECLARE_METATYPE(NavigationController)

#endif // NAVIGATIONCONTROLLER_H
