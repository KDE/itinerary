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
#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include "kandroidextras/contentresolver.h"
#include "kandroidextras/javatypes.h"
#include "kandroidextras/jnisignature.h"
#include "kandroidextras/manifestpermission.h"
using namespace KAndroidExtras;

#include "private/qandroidextras_p.h"
#endif

#include <csignal>

using namespace Qt::Literals;

void DevelopmentModeController::enablePublicTransportLogging()
{
#ifdef Q_OS_ANDROID
    if (QtAndroidPrivate::checkPermission(ManifestPermission::WRITE_EXTERNAL_STORAGE).result() != QtAndroidPrivate::PermissionResult::Authorized) {
        if (QtAndroidPrivate::requestPermission(ManifestPermission::WRITE_EXTERNAL_STORAGE).result() == QtAndroidPrivate::PermissionResult::Authorized) {
            enablePublicTransportLogging();
        } else {
            return;
        }
    }

    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    const auto f = context.callObjectMethod("getExternalFilesDir", Jni::signature<java::io::File(java::lang::String)>(), nullptr);
    const auto baseDir = f.callObjectMethod("getPath", Jni::signature<java::lang::String()>()).toString();
#else
    const auto baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
    qputenv("KPUBLICTRANSPORT_LOG_DIR", (baseDir + QLatin1StringView("/kpublictransport-log")).toUtf8());
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
    dest += QLatin1StringView("/org.kde.kosmindoormap/assets/css/");

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
    path += QLatin1StringView("/org.kde.kosmindoormap/assets/css/");
    QDir(path).removeRecursively();
}

void DevelopmentModeController::clearOsmTileCache()
{
    // see KOSMIndoorMap::TileCache
    QString base;
    if (!qEnvironmentVariableIsSet("KOSMINDOORMAP_CACHE_PATH")) {
        base = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
            + QLatin1StringView("/org.kde.osm/vectorosm");
    } else {
        base = qEnvironmentVariable("KOSMINDOORMAP_CACHE_PATH");
    }
    QDir(base + QLatin1StringView("/17")).removeRecursively();
}

void DevelopmentModeController::retranslate(const QString &locale)
{
    qputenv("LANGUAGE", locale.toUtf8());
    QCoreApplication::sendEvent(QCoreApplication::instance(), new QEvent(QEvent::LocaleChange));
    QCoreApplication::sendEvent(QCoreApplication::instance(), new QEvent(QEvent::LanguageChange));
}

void DevelopmentModeController::crash()
{
    std::raise(SIGSEGV);
}

QString DevelopmentModeController::screenInfo()
{
    QString info;
    const auto screens = QGuiApplication::screens();
    for (auto screen : screens) {
        info += screen->name() + QLatin1Char(' ') + screen->model() + QLatin1Char(' ') + screen->serialNumber() + QLatin1Char('\n');
        info += QLatin1StringView("size: ") + QString::number(screen->size().width()) + QLatin1Char('x') + QString::number(screen->size().height()) + QLatin1Char('\n');
        info += QLatin1StringView("virtual size: ") + QString::number(screen->virtualSize().width()) + QLatin1Char('x') + QString::number(screen->virtualSize().height()) + QLatin1Char('\n');
        info += QLatin1StringView("physical size: ") + QString::number(screen->physicalSize().width()) + QLatin1Char('x') + QString::number(screen->physicalSize().height()) + QLatin1Char('\n');
        info += QLatin1StringView("logical DPI: ") + QString::number(screen->logicalDotsPerInchX())  + QLatin1Char('x') + QString::number(screen->logicalDotsPerInchY()) + QLatin1Char('\n');
        info += QLatin1StringView("physical DPI: ") + QString::number(screen->physicalDotsPerInchX())  + QLatin1Char('x') + QString::number(screen->physicalDotsPerInchY()) + QLatin1Char('\n');
        info += QLatin1StringView("device pixel ratio: ") + QString::number(screen->devicePixelRatio()) + QLatin1Char('\n');
        info += QLatin1Char('\n');
    }
    return info;
}

QString DevelopmentModeController::localeInfo()
{
    QString info;
    info += "locale name: "_L1 + QLocale().name() + '\n'_L1;
    info += "bcp47: "_L1 + QLocale().bcp47Name() + '\n'_L1;
    info += "ui languages: "_L1 + QLocale().uiLanguages().join(", "_L1) + '\n'_L1;
    return info;
}

#include "moc_developmentmodecontroller.cpp"
