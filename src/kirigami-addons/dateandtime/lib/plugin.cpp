/*
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "timeinputvalidator.h"
#include "yearmodel.h"
#include "monthmodel.h"
#include "infinitecalendarviewmodel.h"

#ifdef Q_OS_ANDROID
#include "androidintegration.h"

using namespace KirigamiAddonsDateAndTime;
#endif

class KirigamiAddonsDataAndTimePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    KirigamiAddonsDataAndTimePlugin() = default;
    ~KirigamiAddonsDataAndTimePlugin() = default;
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};

void KirigamiAddonsDataAndTimePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine)
    Q_UNUSED(uri)
}

void KirigamiAddonsDataAndTimePlugin::registerTypes(const char *uri)
{
    qmlRegisterType<YearModel>(uri, 0, 1, "YearModel");
    qmlRegisterType<MonthModel>(uri, 0, 1, "MonthModel");
    qmlRegisterType<TimeInputValidator>(uri, 0, 1, "TimeInputValidator");
    qmlRegisterType<InfiniteCalendarViewModel>(uri, 0, 1, "InfiniteCalendarViewModel");

#ifdef Q_OS_ANDROID
    qmlRegisterSingletonType<AndroidIntegration>(uri, 0, 1, "AndroidIntegration", [](QQmlEngine*, QJSEngine*) -> QObject* {
        QQmlEngine::setObjectOwnership(&AndroidIntegration::instance(), QQmlEngine::CppOwnership);
        return &AndroidIntegration::instance();
    });
#endif
}

#include "plugin.moc"
