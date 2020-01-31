/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "itinerary_version.h"
#include "logging.h"

#include "applicationcontroller.h"
#include "brightnessmanager.h"
#include "countryinformation.h"
#include "countrymodel.h"
#include "documentmanager.h"
#include "documentsmodel.h"
#include "favoritelocationmodel.h"
#include "livedatamanager.h"
#include "localizer.h"
#include "lockmanager.h"
#include "navigationcontroller.h"
#include "pkpassmanager.h"
#include "timelinemodel.h"
#include "pkpassimageprovider.h"
#include "publictransport.h"
#include "reservationmanager.h"
#include "settings.h"
#include "statisticsmodel.h"
#include "statisticstimerangemodel.h"
#include "tickettokenmodel.h"
#include "timelinedelegatecontroller.h"
#include "transfermanager.h"
#include "tripgroupinfoprovider.h"
#include "tripgroupmanager.h"
#include "tripgroupproxymodel.h"
#include "util.h"
#include "weatherforecastmodel.h"

#include <weatherforecastmanager.h>

#include <KItinerary/CountryDb>
#include <KItinerary/Ticket>

#include <KPkPass/Field>
#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>

#ifndef Q_OS_ANDROID
#include <KDBusService>
#endif

#include <KLocalizedContext>
#include <KLocalizedString>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef Q_OS_ANDROID
#include <kandroidextras/activity.h>
#include <kandroidextras/intent.h>

#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QApplication>
#endif

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QWindow>

void handleViewIntent(ApplicationController *appController)
{
#ifdef Q_OS_ANDROID
    // handle opened files
    using namespace KAndroidExtras;
    appController->importFromUrl(Activity::getIntent().getData());
#else
    Q_UNUSED(appController);
#endif
}

void handlePositionalArguments(ApplicationController *appController, const QStringList &args)
{
    for (const auto &file : args) {
        const auto localUrl = QUrl::fromLocalFile(file);
        if (QFile::exists(localUrl.toLocalFile()))
            appController->importFromUrl(localUrl);
        else
            appController->importFromUrl(QUrl::fromUserInput(file));
    }
}

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("itinerary"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral(ITINERARY_VERSION_STRING));

    QGuiApplication::setApplicationDisplayName(i18n("KDE Itinerary"));
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv); // for native file dialogs
#endif
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("map-globe")));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"), i18n("PkPass or JSON-LD file to import."));
    parser.process(app);

#ifndef Q_OS_ANDROID
    KDBusService service(KDBusService::Unique);
#endif

    Settings settings;
    PkPassManager passMgr;
    ReservationManager resMgr;
    DocumentManager docMgr;
    resMgr.setPkPassManager(&passMgr);
    TripGroupManager tripGroupMgr;
    tripGroupMgr.setReservationManager(&resMgr);
    ApplicationController appController;
    appController.setReservationManager(&resMgr);
    appController.setPkPassManager(&passMgr);
    appController.setDocumentManager(&docMgr);
    BrightnessManager brightnessManager;
    LockManager lockManager;

    LiveDataManager liveDataMgr;
    liveDataMgr.setPkPassManager(&passMgr);
    liveDataMgr.setReservationManager(&resMgr);
    liveDataMgr.setPollingEnabled(settings.queryLiveData());
    QObject::connect(&settings, &Settings::queryLiveDataChanged, &liveDataMgr, &LiveDataManager::setPollingEnabled);

#ifndef Q_OS_ANDROID
    QObject::connect(&service, &KDBusService::activateRequested, [&parser, &appController](const QStringList &args, const QString &workingDir) {
        qCDebug(Log) << "remote activation" << args << workingDir;
        if (!args.isEmpty()) {
            QDir::setCurrent(workingDir);
            parser.parse(args);
            handlePositionalArguments(&appController, parser.positionalArguments());
        }
        if (!QGuiApplication::allWindows().isEmpty()) {
            QGuiApplication::allWindows().at(0)->requestActivate();
        }
    });
#endif

    TimelineModel timelineModel;
    timelineModel.setHomeCountryIsoCode(settings.homeCountryIsoCode());
    timelineModel.setReservationManager(&resMgr);
    QObject::connect(&settings, &Settings::homeCountryIsoCodeChanged, &timelineModel, &TimelineModel::setHomeCountryIsoCode);

    WeatherForecastManager weatherForecastMgr;
    weatherForecastMgr.setAllowNetworkAccess(settings.weatherForecastEnabled());
    QObject::connect(&settings, &Settings::weatherForecastEnabledChanged, &weatherForecastMgr, &WeatherForecastManager::setAllowNetworkAccess);
    timelineModel.setWeatherForecastManager(&weatherForecastMgr);
    timelineModel.setTripGroupManager(&tripGroupMgr);

    TripGroupProxyModel tripGroupProxy;
    tripGroupProxy.setSourceModel(&timelineModel);

    TripGroupInfoProvider tripGroupInfoProvider;
    tripGroupInfoProvider.setReservationManager(&resMgr);
    tripGroupInfoProvider.setWeatherForecastManager(&weatherForecastMgr);

    TransferManager transferManager;
    transferManager.setReservationManager(&resMgr);
    transferManager.setTripGroupManager(&tripGroupMgr);
    timelineModel.setTransferManager(&transferManager);

    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});
    qmlRegisterUncreatableType<KPkPass::Pass>("org.kde.pkpass", 1, 0, "Pass", {});
    qmlRegisterUncreatableType<KPkPass::BoardingPass>("org.kde.pkpass", 1, 0, "BoardingPass", {});

    qRegisterMetaType<KItinerary::KnowledgeDb::DrivingSide>();
    qmlRegisterUncreatableType<KItinerary::Ticket>("org.kde.kitinerary", 1, 0, "Ticket", {});
    qmlRegisterUncreatableMetaObject(KItinerary::KnowledgeDb::staticMetaObject, "org.kde.kitinerary", 1, 0, "KnowledgeDb", {});

    qRegisterMetaType<ReservationManager*>();
    qmlRegisterUncreatableType<CountryInformation>("org.kde.itinerary", 1, 0, "CountryInformation", {});
    qmlRegisterType<CountryModel>("org.kde.itinerary", 1, 0, "CountryModel");
    qmlRegisterType<DocumentsModel>("org.kde.itinerary", 1, 0, "DocumentsModel");
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "Localizer", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(Localizer());
    });
    qmlRegisterType<TicketTokenModel>("org.kde.itinerary", 1, 0, "TicketTokenModel");
    qmlRegisterUncreatableType<TimelineElement>("org.kde.itinerary", 1, 0, "TimelineElement", {});
    qmlRegisterUncreatableType<TimelineModel>("org.kde.itinerary", 1, 0, "TimelineModel", {});
    qmlRegisterType<TimelineDelegateController>("org.kde.itinerary", 1, 0, "TimelineDelegateController");
    qRegisterMetaType<Transfer::Alignment>();
    qmlRegisterUncreatableType<Transfer>("org.kde.itinerary", 1, 0, "Transfer", {});
    qmlRegisterSingletonType<DocumentManager>("org.kde.itinerary", 1, 0, "TransferManager", [](QQmlEngine *engine, QJSEngine*) -> QObject* {
        engine->setObjectOwnership(TransferManager::instance(), QQmlEngine::CppOwnership);
        return TransferManager::instance();
    });
    qmlRegisterType<FavoriteLocationModel>("org.kde.itinerary", 1, 0, "FavoriteLocationModel");
    qmlRegisterSingletonType<Util>("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine*, QJSEngine*) -> QObject*{
        return new Util;
    });
    qRegisterMetaType<WeatherForecast>();
    qmlRegisterType<WeatherForecastModel>("org.kde.itinerary", 1, 0, "WeatherForecastModel");
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PublicTransport", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(PublicTransport());
    });
    qmlRegisterSingletonType<DocumentManager>("org.kde.itinerary", 1, 0, "ApplicationController", [](QQmlEngine *engine, QJSEngine*) -> QObject* {
        engine->setObjectOwnership(ApplicationController::instance(), QQmlEngine::CppOwnership);
        return ApplicationController::instance();
    });
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "NavigationController", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(NavigationController());
    });
    qmlRegisterSingletonType<DocumentManager>("org.kde.itinerary", 1, 0, "DocumentManager", [](QQmlEngine *engine, QJSEngine*) -> QObject* {
        engine->setObjectOwnership(DocumentManager::instance(), QQmlEngine::CppOwnership);
        return DocumentManager::instance();
    });
    qmlRegisterUncreatableType<StatisticsItem>("org.kde.itinerary", 1, 0, "StatisticsItem", {});
    qmlRegisterType<StatisticsModel>("org.kde.itinerary", 1, 0, "StatisticsModel");
    qmlRegisterType<StatisticsTimeRangeModel>("org.kde.itinerary", 1, 0, "StatisticsTimeRangeModel");

    qmlRegisterType<QSortFilterProxyModel>("org.kde.itinerary", 1, 0, "SortFilterProxyModel");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("_pkpassManager"), &passMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_reservationManager"), &resMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_timelineModel"), &tripGroupProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("_settings"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("_weatherForecastManager"), &weatherForecastMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_brightnessManager"), &brightnessManager);
    engine.rootContext()->setContextProperty(QStringLiteral("_lockManager"), &lockManager);
    engine.rootContext()->setContextProperty(QStringLiteral("_liveDataManager"), &liveDataMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_tripGroupInfoProvider"), QVariant::fromValue(tripGroupInfoProvider));
    engine.load(QStringLiteral("qrc:/main.qml"));

    handlePositionalArguments(&appController, parser.positionalArguments());
    handleViewIntent(&appController);

    return app.exec();
}
