/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "developmentmodecontroller.h"

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include <kandroidextras/contentresolver.h>
#include <kandroidextras/javatypes.h>
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/manifestpermission.h>
using namespace KAndroidExtras;

#include <QtAndroid>
#endif

#include <csignal>

void DevelopmentModeController::enablePublicTransportLogging()
{
#ifdef Q_OS_ANDROID
    if (QtAndroid::checkPermission(ManifestPermission::WRITE_EXTERNAL_STORAGE) != QtAndroid::PermissionResult::Granted) {
        QtAndroid::requestPermissions({ManifestPermission::WRITE_EXTERNAL_STORAGE}, [this] (const QtAndroid::PermissionResultMap &result){
            if (result[ManifestPermission::WRITE_EXTERNAL_STORAGE] == QtAndroid::PermissionResult::Granted) {
                enablePublicTransportLogging();
            }
        });
        return;
    }

    const auto f = QtAndroid::androidContext().callObjectMethod("getExternalFilesDir", Jni::signature<java::io::File(java::lang::String)>(), nullptr);
    const auto baseDir = f.callObjectMethod("getPath", Jni::signature<java::lang::String()>()).toString();
#else
    const auto baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
    qputenv("KPUBLICTRANSPORT_LOG_DIR", (baseDir + QLatin1String("/kpublictransport-log")).toUtf8());
}

void DevelopmentModeController::importMapCSS(const QUrl &url)
{
    const auto src = url.isLocalFile() ? url.toLocalFile() : url.toString();

#ifndef Q_OS_ANDROID
    const auto fileName = QFileInfo(src).fileName();
    auto dest = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
#else
    const auto fileName = KAndroidExtras::ContentResolver::fileName(url);
    auto dest = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
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
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#endif
    path += QLatin1String("/org.kde.kosmindoormap/assets/css/");
    QDir(path).removeRecursively();
}

void DevelopmentModeController::clearOsmTileCache()
{
    // see KOSMIndoorMap::TileCache
    QString base;
    if (!qEnvironmentVariableIsSet("KOSMINDOORMAP_CACHE_PATH")) {
        base = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
            + QLatin1String("/org.kde.osm/vectorosm");
    } else {
        base = qEnvironmentVariable("KOSMINDOORMAP_CACHE_PATH");
    }
    QDir(base + QLatin1String("/17")).removeRecursively();
}

void DevelopmentModeController::crash()
{
    std::raise(SIGSEGV);
}
