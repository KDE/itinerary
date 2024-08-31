/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DEVELOPMENTMODECONTROLLER_H
#define DEVELOPMENTMODECONTROLLER_H

#include <QMetaType>
#include <QUrl>

/** Functions for the development mode. */
class DevelopmentModeController
{
    Q_GADGET
public:
    Q_INVOKABLE void enablePublicTransportLogging();
    Q_INVOKABLE void importMapCSS(const QUrl &url);
    Q_INVOKABLE void purgeMapCSS();
    Q_INVOKABLE void clearOsmTileCache();
    Q_INVOKABLE void crash();
    Q_INVOKABLE [[nodiscard]] static QString screenInfo();
    Q_INVOKABLE [[nodiscard]] static QString localeInfo();
};

Q_DECLARE_METATYPE(DevelopmentModeController)

#endif // DEVELOPMENTMODECONTROLLER_H
