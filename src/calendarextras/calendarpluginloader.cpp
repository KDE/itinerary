/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarpluginloader.h"

#ifdef Q_OS_ANDROID
#include "androidcalendarplugin.h"
#endif

using namespace KCalendarCore;

struct PluginLoader {
    PluginLoader();
    std::unique_ptr<KCalendarCore::CalendarPlugin> plugin;
};

PluginLoader::PluginLoader()
{
#ifdef Q_OS_ANDROID
    plugin.reset(new AndroidCalendarPlugin(nullptr, {}));
#else
    // TODO
#endif
}

Q_GLOBAL_STATIC(PluginLoader, s_pluginLoader)

bool CalendarPluginLoader::hasPlugin()
{
    return (bool)s_pluginLoader->plugin;
}

KCalendarCore::CalendarPlugin* CalendarPluginLoader::plugin()
{
    return s_pluginLoader->plugin.get();
}
