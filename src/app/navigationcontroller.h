/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    Q_INVOKABLE void showOnMap(float latitude, float longitude, int zoom);
    Q_INVOKABLE void showOnWheelmap(float latitude, float longitude);

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
