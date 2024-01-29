/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "solidextras/brightnessmanager.h"
#include "solidextras/lockmanager.h"
#include "solidextras/networkstatus.h"

class SolidExtrasQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
    void registerTypes(const char* uri) override;
};

using namespace SolidExtras;

void SolidExtrasQmlPlugin::registerTypes(const char*)
{
    qmlRegisterSingletonType<NetworkStatus>("org.kde.solidextras", 1, 0, "BrightnessManager", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new BrightnessManager;
    });
    qmlRegisterSingletonType<NetworkStatus>("org.kde.solidextras", 1, 0, "LockManager", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new LockManager;
    });
    qmlRegisterSingletonType<NetworkStatus>("org.kde.solidextras", 1, 0, "NetworkStatus", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new NetworkStatus;
    });
}

#include "solidextrasqmlplugin.moc"
