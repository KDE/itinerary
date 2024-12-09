/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "solidextras/brightnessmanager.h"
#include "solidextras/lockmanager.h"

class SolidExtrasQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
    void registerTypes(const char *uri) override;
};

void SolidExtrasQmlPlugin::registerTypes(const char *)
{
    qmlRegisterSingletonType<BrightnessManager>("org.kde.solidextras", 1, 0, "BrightnessManager", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new BrightnessManager;
    });
    qmlRegisterSingletonType<LockManager>("org.kde.solidextras", 1, 0, "LockManager", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new LockManager;
    });
}

#include "solidextrasqmlplugin.moc"
