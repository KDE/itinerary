/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "migrator.h"
#include "logging.h"

#include <QSettings>

void Migrator::run()
{
    QSettings settings;
    auto version = settings.value("Version", 0).toInt();
    qCDebug(Log) << "Migrating from version" << version;

    switch (version) {
        case 0:
            dropTripGroupExpandCollapseState();
            ++version;
            // add future updates here with [[fallthrough]]

            break;
        default:
            // already up to date
            qCDebug(Log) << "Aleady on current version, nothing to do.";
            return;
    }

    qCDebug(Log) << "Migration done to version" << version;
    settings.setValue("Version", version);
}

void Migrator::dropTripGroupExpandCollapseState()
{
    // remove leftover collapse/expand state from the old TripGroupProxyModel
    QSettings settings;
    settings.remove("TripGroupProxyState");
}
