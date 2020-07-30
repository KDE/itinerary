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
#include "countrymodel.h"
#include "documentmanager.h"
#include "documentsmodel.h"
#include "favoritelocationmodel.h"
#include "livedatamanager.h"
#include "localizer.h"
#include "locationinformation.h"
#include "navigationcontroller.h"
#include "notificationconfigcontroller.h"
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
#include <KIdleInhibition>

#include <QQuickStyle>
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

void registerKPkPassTypes()
{
    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});
    qmlRegisterUncreatableType<KPkPass::Pass>("org.kde.pkpass", 1, 0, "Pass", {});
    qmlRegisterUncreatableType<KPkPass::BoardingPass>("org.kde.pkpass", 1, 0, "BoardingPass", {});
}

void registerKItineraryTypes()
{
    qRegisterMetaType<KItinerary::KnowledgeDb::DrivingSide>();
    qmlRegisterUncreatableType<KItinerary::Ticket>("org.kde.kitinerary", 1, 0, "Ticket", {});
    qmlRegisterUncreatableMetaObject(KItinerary::KnowledgeDb::staticMetaObject, "org.kde.kitinerary", 1, 0, "KnowledgeDb", {});
}

void registerApplicationTypes()
{
    qRegisterMetaType<ReservationManager*>();
    qRegisterMetaType<Transfer::Alignment>();
    qRegisterMetaType<TripGroupManager*>();
    qRegisterMetaType<WeatherForecast>();

    qmlRegisterUncreatableType<LocationInformation>("org.kde.itinerary", 1, 0, "LocationInformation", {});
    qmlRegisterUncreatableType<StatisticsItem>("org.kde.itinerary", 1, 0, "StatisticsItem", {});
    qmlRegisterUncreatableType<TimelineElement>("org.kde.itinerary", 1, 0, "TimelineElement", {});
    qmlRegisterUncreatableType<TimelineModel>("org.kde.itinerary", 1, 0, "TimelineModel", {});
    qmlRegisterUncreatableType<Transfer>("org.kde.itinerary", 1, 0, "Transfer", {});

    qmlRegisterType<CountryModel>("org.kde.itinerary", 1, 0, "CountryModel");
    qmlRegisterType<DocumentsModel>("org.kde.itinerary", 1, 0, "DocumentsModel");
    qmlRegisterType<QSortFilterProxyModel>("org.kde.itinerary", 1, 0, "SortFilterProxyModel"); // TODO use this from kitemmodels?
    qmlRegisterType<StatisticsModel>("org.kde.itinerary", 1, 0, "StatisticsModel");
    qmlRegisterType<StatisticsTimeRangeModel>("org.kde.itinerary", 1, 0, "StatisticsTimeRangeModel");
    qmlRegisterType<TicketTokenModel>("org.kde.itinerary", 1, 0, "TicketTokenModel");
    qmlRegisterType<TimelineDelegateController>("org.kde.itinerary", 1, 0, "TimelineDelegateController");
    qmlRegisterType<WeatherForecastModel>("org.kde.itinerary", 1, 0, "WeatherForecastModel");
}

// for registering QML singletons only
static DocumentManager *s_documentManager = nullptr;
static FavoriteLocationModel *s_favoriteLocationModel = nullptr;
static PkPassManager *s_pkPassManager = nullptr;
static Settings *s_settings = nullptr;
static TransferManager *s_tranferManager = nullptr;
static TripGroupManager *s_tripGroupManager = nullptr;
static LiveDataManager *s_liveDataMnager = nullptr;
static WeatherForecastManager *s_weatherForecastManager = nullptr;

#define REGISTER_SINGLETON_INSTANCE(Class, Instance) \
    qmlRegisterSingletonType<Class>("org.kde.itinerary", 1, 0, #Class, [](QQmlEngine *engine, QJSEngine*) -> QObject* { \
        engine->setObjectOwnership(Instance, QQmlEngine::CppOwnership); \
        return Instance; \
    });

#define REGISTER_SINGLETON_GADGET(Class) \
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, #Class, [](QQmlEngine*, QJSEngine *engine) -> QJSValue { \
        return engine->toScriptValue(Class()); \
    });

void registerApplicationSingletons()
{
    REGISTER_SINGLETON_INSTANCE(ApplicationController, ApplicationController::instance())
    REGISTER_SINGLETON_INSTANCE(DocumentManager, s_documentManager)
    REGISTER_SINGLETON_INSTANCE(FavoriteLocationModel, s_favoriteLocationModel)
    REGISTER_SINGLETON_INSTANCE(PkPassManager, s_pkPassManager)
    REGISTER_SINGLETON_INSTANCE(Settings, s_settings)
    REGISTER_SINGLETON_INSTANCE(TransferManager, s_tranferManager)
    REGISTER_SINGLETON_INSTANCE(TripGroupManager, s_tripGroupManager)
    REGISTER_SINGLETON_INSTANCE(LiveDataManager, s_liveDataMnager)
    REGISTER_SINGLETON_INSTANCE(WeatherForecastManager, s_weatherForecastManager)

    REGISTER_SINGLETON_GADGET(Localizer)
    REGISTER_SINGLETON_GADGET(NavigationController)
    REGISTER_SINGLETON_GADGET(NotificationConfigController)
    REGISTER_SINGLETON_GADGET(PublicTransport)

    qmlRegisterSingletonType<Util>("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine*, QJSEngine*) -> QObject*{
        return new Util;
    });
}

#undef REGISTER_SINGLETON_INSTANCE

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
    QGuiApplication::setDesktopFileName(QStringLiteral("org.kde.itinerary"));
    QCoreApplication::setApplicationVersion(QStringLiteral(ITINERARY_VERSION_STRING));

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QApplication app(argc, argv); // for native file dialogs
#endif
    QGuiApplication::setApplicationDisplayName(i18n("KDE Itinerary"));
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("itinerary")));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"), i18n("PkPass or JSON-LD file to import."));
    parser.process(app);

#ifndef Q_OS_ANDROID
    KDBusService service(KDBusService::Unique);
#endif

    Settings settings;
    s_settings = &settings;

    PkPassManager passMgr;
    s_pkPassManager = &passMgr;

    ReservationManager resMgr;
    resMgr.setPkPassManager(&passMgr);

    DocumentManager docMgr;
    s_documentManager = &docMgr;

    FavoriteLocationModel favLocModel;
    s_favoriteLocationModel = &favLocModel;

    TripGroupManager tripGroupMgr;
    tripGroupMgr.setReservationManager(&resMgr);
    s_tripGroupManager = &tripGroupMgr;

    BrightnessManager brightnessManager;
    KIdleInhibition lockManager;

    LiveDataManager liveDataMgr;
    liveDataMgr.setPkPassManager(&passMgr);
    liveDataMgr.setReservationManager(&resMgr);
    liveDataMgr.setPollingEnabled(settings.queryLiveData());
    liveDataMgr.setShowNotificationsOnLockScreen(settings.showNotificationOnLockScreen());
    QObject::connect(&settings, &Settings::queryLiveDataChanged, &liveDataMgr, &LiveDataManager::setPollingEnabled);
    QObject::connect(&settings, &Settings::showNotificationOnLockScreenChanged, &liveDataMgr, &LiveDataManager::setShowNotificationsOnLockScreen);
    s_liveDataMnager = &liveDataMgr;

    WeatherForecastManager weatherForecastMgr;
    weatherForecastMgr.setAllowNetworkAccess(settings.weatherForecastEnabled());
    QObject::connect(&settings, &Settings::weatherForecastEnabledChanged, &weatherForecastMgr, &WeatherForecastManager::setAllowNetworkAccess);
    s_weatherForecastManager = &weatherForecastMgr;

    TransferManager transferManager;
    transferManager.setReservationManager(&resMgr);
    transferManager.setTripGroupManager(&tripGroupMgr);
    transferManager.setFavoriteLocationModel(&favLocModel);
    transferManager.setPublicTransportManager(liveDataMgr.publicTransportManager());
    transferManager.setAutoAddTransfers(settings.autoAddTransfers());
    transferManager.setAutoFillTransfers(settings.autoFillTransfers());
    QObject::connect(&settings, &Settings::autoAddTransfersChanged, &transferManager, &TransferManager::setAutoAddTransfers);
    QObject::connect(&settings, &Settings::autoFillTransfersChanged, &transferManager, &TransferManager::setAutoFillTransfers);
    s_tranferManager = &transferManager;

    TimelineModel timelineModel;
    timelineModel.setHomeCountryIsoCode(settings.homeCountryIsoCode());
    timelineModel.setReservationManager(&resMgr);
    timelineModel.setWeatherForecastManager(&weatherForecastMgr);
    timelineModel.setTransferManager(&transferManager);
    timelineModel.setTripGroupManager(&tripGroupMgr);
    QObject::connect(&settings, &Settings::homeCountryIsoCodeChanged, &timelineModel, &TimelineModel::setHomeCountryIsoCode);

    TripGroupProxyModel tripGroupProxy;
    tripGroupProxy.setSourceModel(&timelineModel);

    TripGroupInfoProvider tripGroupInfoProvider;
    tripGroupInfoProvider.setReservationManager(&resMgr);
    tripGroupInfoProvider.setWeatherForecastManager(&weatherForecastMgr);

    ApplicationController appController;
    appController.setReservationManager(&resMgr);
    appController.setPkPassManager(&passMgr);
    appController.setDocumentManager(&docMgr);
    appController.setFavoriteLocationModel(&favLocModel);
    appController.setTransferManager(&transferManager);
    appController.setLiveDataManager(&liveDataMgr);
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

    registerKPkPassTypes();
    registerKItineraryTypes();
    registerApplicationTypes();
    registerApplicationSingletons();

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    auto l10nContext = new KLocalizedContext(&engine);
    l10nContext->setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));
    engine.rootContext()->setContextObject(l10nContext);
    // TODO get rid of those, e.g. by using singletons
    engine.rootContext()->setContextProperty(QStringLiteral("_reservationManager"), &resMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_timelineModel"), &tripGroupProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("_brightnessManager"), &brightnessManager);
    engine.rootContext()->setContextProperty(QStringLiteral("_lockManager"), &lockManager);
    engine.rootContext()->setContextProperty(QStringLiteral("_tripGroupInfoProvider"), QVariant::fromValue(tripGroupInfoProvider));
    engine.load(QStringLiteral("qrc:/main.qml"));

    handlePositionalArguments(&appController, parser.positionalArguments());
    handleViewIntent(&appController);

    return app.exec();
}
