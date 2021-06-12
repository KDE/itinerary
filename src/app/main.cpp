/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "itinerary_version.h"
#include "logging.h"

#include "applicationcontroller.h"
#include "countrymodel.h"
#include "developmentmodecontroller.h"
#include "documentmanager.h"
#include "documentsmodel.h"
#include "favoritelocationmodel.h"
#include "livedatamanager.h"
#include "localizer.h"
#include "locationinformation.h"
#include "mapdownloadmanager.h"
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

#include <KAboutData>
#if HAVE_KCRASH
#include <KCrash>
#endif

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
static ReservationManager *s_reservationManager = nullptr;
static DocumentManager *s_documentManager = nullptr;
static FavoriteLocationModel *s_favoriteLocationModel = nullptr;
static PkPassManager *s_pkPassManager = nullptr;
static Settings *s_settings = nullptr;
static TransferManager *s_tranferManager = nullptr;
static TripGroupManager *s_tripGroupManager = nullptr;
static LiveDataManager *s_liveDataMnager = nullptr;
static WeatherForecastManager *s_weatherForecastManager = nullptr;
static TripGroupInfoProvider s_tripGroupInfoProvider;
static TripGroupProxyModel *s_tripGroupProxyModel = nullptr;
static MapDownloadManager *s_mapDownloadManager = nullptr;

#define REGISTER_SINGLETON_INSTANCE(Class, Instance) \
    qmlRegisterSingletonType<Class>("org.kde.itinerary", 1, 0, #Class, [](QQmlEngine *engine, QJSEngine*) -> QObject* { \
        engine->setObjectOwnership(Instance, QQmlEngine::CppOwnership); \
        return Instance; \
    });

#define REGISTER_SINGLETON_GADGET_INSTANCE(Class, Instance) \
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, #Class, [](QQmlEngine *engine, QJSEngine*) -> QJSValue { \
        return engine->toScriptValue(Instance); \
    });

#define REGISTER_SINGLETON_GADGET_FACTORY(Class) \
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, #Class, [](QQmlEngine*, QJSEngine *engine) -> QJSValue { \
        return engine->toScriptValue(Class()); \
    });

void registerApplicationSingletons()
{
    REGISTER_SINGLETON_INSTANCE(ApplicationController, ApplicationController::instance())
    REGISTER_SINGLETON_INSTANCE(ReservationManager, s_reservationManager)
    REGISTER_SINGLETON_INSTANCE(DocumentManager, s_documentManager)
    REGISTER_SINGLETON_INSTANCE(FavoriteLocationModel, s_favoriteLocationModel)
    REGISTER_SINGLETON_INSTANCE(PkPassManager, s_pkPassManager)
    REGISTER_SINGLETON_INSTANCE(Settings, s_settings)
    REGISTER_SINGLETON_INSTANCE(TransferManager, s_tranferManager)
    REGISTER_SINGLETON_INSTANCE(TripGroupManager, s_tripGroupManager)
    REGISTER_SINGLETON_INSTANCE(LiveDataManager, s_liveDataMnager)
    REGISTER_SINGLETON_INSTANCE(WeatherForecastManager, s_weatherForecastManager)
    REGISTER_SINGLETON_INSTANCE(TripGroupProxyModel, s_tripGroupProxyModel)
    REGISTER_SINGLETON_INSTANCE(MapDownloadManager, s_mapDownloadManager)

    REGISTER_SINGLETON_GADGET_INSTANCE(TripGroupInfoProvider, s_tripGroupInfoProvider)

    REGISTER_SINGLETON_GADGET_FACTORY(DevelopmentModeController)
    REGISTER_SINGLETON_GADGET_FACTORY(Localizer)
    REGISTER_SINGLETON_GADGET_FACTORY(NavigationController)
    REGISTER_SINGLETON_GADGET_FACTORY(NotificationConfigController)
    REGISTER_SINGLETON_GADGET_FACTORY(PublicTransport)

    qmlRegisterSingletonType<Util>("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine*, QJSEngine*) -> QObject*{
        return new Util;
    });
}

#undef REGISTER_SINGLETON_INSTANCE

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

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QApplication app(argc, argv); // for native file dialogs

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif
    QGuiApplication::setApplicationDisplayName(i18n("KDE Itinerary"));
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("itinerary")));

    auto aboutData = KAboutData::applicationData();
    aboutData.setProductName("KDE Itinerary/general"); // Bugzilla product/component name
    aboutData.setLicense(KAboutLicense::LGPL);
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("file"), i18n("PkPass or JSON-LD file to import."));
    parser.process(app);
    aboutData.processCommandLine(&parser);
#if HAVE_KCRASH
    KCrash::initialize();
#endif

#ifndef Q_OS_ANDROID
    KDBusService service(KDBusService::Unique);
#endif

    Settings settings;
    s_settings = &settings;

    PkPassManager passMgr;
    s_pkPassManager = &passMgr;

    ReservationManager resMgr;
    resMgr.setPkPassManager(&passMgr);
    s_reservationManager = &resMgr;

    DocumentManager docMgr;
    s_documentManager = &docMgr;

    FavoriteLocationModel favLocModel;
    s_favoriteLocationModel = &favLocModel;

    TripGroupManager tripGroupMgr;
    tripGroupMgr.setReservationManager(&resMgr);
    s_tripGroupManager = &tripGroupMgr;

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
    s_tripGroupProxyModel = &tripGroupProxy;

    s_tripGroupInfoProvider.setReservationManager(&resMgr);
    s_tripGroupInfoProvider.setWeatherForecastManager(&weatherForecastMgr);

    MapDownloadManager mapDownloadMgr;
    mapDownloadMgr.setReservationManager(&resMgr);
    mapDownloadMgr.setAutomaticDownloadEnabled(s_settings->preloadMapData());
    QObject::connect(s_settings, &Settings::preloadMapDataChanged, &mapDownloadMgr, &MapDownloadManager::setAutomaticDownloadEnabled);
    s_mapDownloadManager = &mapDownloadMgr;

    ApplicationController appController;
    appController.setReservationManager(&resMgr);
    appController.setPkPassManager(&passMgr);
    appController.setDocumentManager(&docMgr);
    appController.setFavoriteLocationModel(&favLocModel);
    appController.setTransferManager(&transferManager);
    appController.setLiveDataManager(&liveDataMgr);
    appController.setTripGroupManager(&tripGroupMgr);
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
    engine.load(QStringLiteral("qrc:/main.qml"));

    handlePositionalArguments(&appController, parser.positionalArguments());
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    appController.importFromIntent(Activity::getIntent());
#endif

    return app.exec();
}
