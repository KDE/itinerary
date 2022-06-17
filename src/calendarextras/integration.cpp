/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QPluginLoader>

#ifdef Q_OS_ANDROID
#include "androidcalendarplugin.h"
Q_IMPORT_PLUGIN(AndroidCalendarPlugin)
#endif
