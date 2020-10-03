/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "developmentmodecontroller.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include <kandroidextras/contentresolver.h>
#endif

void DevelopmentModeController::importMapCSS(const QUrl &url)
{
    const auto src = url.isLocalFile() ? url.toLocalFile() : url.toString();

#ifndef Q_OS_ANDROID
    const auto fileName = QFileInfo(src).fileName();
    auto dest = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
#else
    const auto fileName = KAndroidExtras::ContentResolver::fileName(url);
    auto dest = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    dest += QLatin1String("/org.kde.kosmindoormap/assets/css/");

    qDebug() << "importing mapcss" << url << dest << fileName;
    QDir().mkpath(dest);
    QFile::copy(src, dest + fileName);
}

void DevelopmentModeController::purgeMapCSS()
{
#ifndef Q_OS_ANDROID
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
#else
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    path += QLatin1String("/org.kde.kosmindoormap/assets/css/");
    QDir(path).removeRecursively();
}
