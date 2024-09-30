/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "itinerary_version.h"
#include "logging.h"

#include "applicationcontroller.h"
#include "countrysubdivisionmodel.h"
#include "clipboard.h"
#include "developmentmodecontroller.h"
#include "documentmanager.h"
#include "documentsmodel.h"
#include "factory.h"
#include "favoritelocationmodel.h"
#include "genericpkpass.h"
#include "healthcertificatemanager.h"
#include "importcontroller.h"
#include "intenthandler.h"
#include "journeysectionmodel.h"
#include "kdeconnect.h"
#include "livedatamanager.h"
#include "localizer.h"
#include "locationinformation.h"
#include "mapdownloadmanager.h"
#include "matrixcontroller.h"
#include "navigationcontroller.h"
#include "notificationconfigcontroller.h"
#include "onlineticketimporter.h"
#include "passmanager.h"
#include "pkpassmanager.h"
#include "permissionmanager.h"
#include "pkpassimageprovider.h"
#include "publictransport.h"
#include "reservationhelper.h"
#include "reservationmanager.h"
#include "settings.h"
#include "statisticsmodel.h"
#include "statisticstimerangemodel.h"
#include "tickettokenmodel.h"
#include "timelinedelegatecontroller.h"
#include "timelinemodel.h"
#include "timelinesectiondelegatecontroller.h"
#include "traewellingcontroller.h"
#include "transferdelegatecontroller.h"
#include "transfermanager.h"
#include "tripgroupcontroller.h"
#include "tripgroupfilterproxymodel.h"
#include "tripgroupmanager.h"
#include "tripgroupmodel.h"
#include "tripgroupproxymodel.h"
#include "tripgroupsplitmodel.h"
#include "unitconversion.h"
#include "util.h"
#include "weatherforecastmodel.h"

#if HAVE_MATRIX
#include "matrix/matrixroomsmodel.h"
#include "matrix/matrixbeacon.h"
#endif

#include "weatherforecastmanager.h"

#include <KItinerary/CountryDb>
#include <KItinerary/DocumentUtil>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PriceUtil>
#include <KItinerary/Ticket>

#include <KPkPass/Field>
#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>

#ifndef Q_OS_ANDROID
#include <KDBusService>
#include <KWindowSystem>
#else
#include <KColorSchemeManager>
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
#include "kandroidextras/activity.h"
#include "kandroidextras/intent.h"
#else
#include <QApplication>
#endif

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QWindow>

using namespace Qt::Literals::StringLiterals;

#if !HAVE_MATRIX
class MatrixBeaconStub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant connection MEMBER m_connection)
    QVariant m_connection;
};
#endif

void registerKPkPassTypes()
{
    qmlRegisterUncreatableMetaObject(KPkPass::Barcode::staticMetaObject, "org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableMetaObject(KPkPass::Field::staticMetaObject, "org.kde.pkpass", 1, 0, "Field", {});
    qmlRegisterUncreatableType<KPkPass::Pass>("org.kde.pkpass", 1, 0, "Pass", {});
    qmlRegisterUncreatableType<KPkPass::BoardingPass>("org.kde.pkpass", 1, 0, "BoardingPass", {});
}

void registerKItineraryTypes()
{
    qRegisterMetaType<KItinerary::KnowledgeDb::DrivingSide>();
    qmlRegisterUncreatableMetaObject(KItinerary::Ticket::staticMetaObject, "org.kde.kitinerary", 1, 0, "Ticket", {});
    qmlRegisterUncreatableMetaObject(KItinerary::KnowledgeDb::staticMetaObject, "org.kde.kitinerary", 1, 0, "KnowledgeDb", {});
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PriceUtil", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(KItinerary::PriceUtil());
    });
}

void registerApplicationTypes()
{
    qRegisterMetaType<ReservationManager*>();
    qRegisterMetaType<Transfer::Alignment>();
    qRegisterMetaType<TripGroupManager*>();
    qRegisterMetaType<WeatherForecast>();
    qRegisterMetaType<Permission::Permission>();
    qRegisterMetaType<HealthCertificateManager*>();

    qmlRegisterUncreatableMetaObject(LocationInformation::staticMetaObject, "org.kde.itinerary", 1, 0, "LocationInformation", {});
    qmlRegisterUncreatableMetaObject(StatisticsItem::staticMetaObject, "org.kde.itinerary", 1, 0, "StatisticsItem", {});
    qmlRegisterUncreatableMetaObject(TimelineElement::staticMetaObject, "org.kde.itinerary", 1, 0, "TimelineElement", {});
    qmlRegisterUncreatableMetaObject(Transfer::staticMetaObject, "org.kde.itinerary", 1, 0, "Transfer", {});

    qmlRegisterUncreatableMetaObject(Permission::staticMetaObject, "org.kde.itinerary", 1, 0, "Permission", {});
#if HAVE_MATRIX
    qmlRegisterUncreatableMetaObject(MatrixRoomsModel::staticMetaObject, "org.kde.itinerary", 1, 0, "MatrixRoomsModel", {});
#endif

    qmlRegisterType<CountrySubdivisionModel>("org.kde.itinerary", 1, 0, "CountrySubdivisionModel");
    qmlRegisterType<DocumentsModel>("org.kde.itinerary", 1, 0, "DocumentsModel");
    qmlRegisterType<JourneySectionModel>("org.kde.itinerary", 1, 0, "JourneySectionModel");
    qmlRegisterType<KDEConnectDeviceModel>("org.kde.itinerary", 1, 0, "KDEConnectDeviceModel");
    qmlRegisterType<StatisticsModel>("org.kde.itinerary", 1, 0, "StatisticsModel");
    qmlRegisterType<StatisticsTimeRangeModel>("org.kde.itinerary", 1, 0, "StatisticsTimeRangeModel");
    qmlRegisterType<TicketTokenModel>("org.kde.itinerary", 1, 0, "TicketTokenModel");
    qmlRegisterType<TimelineDelegateController>("org.kde.itinerary", 1, 0, "TimelineDelegateController");
    qmlRegisterType<TimelineModel>("org.kde.itinerary", 1, 0, "TimelineModel");
    qmlRegisterType<TimelineSectionDelegateController>("org.kde.itinerary", 1, 0, "TimelineSectionDelegateController");
    qmlRegisterType<TransferDelegateController>("org.kde.itinerary", 1, 0, "TransferDelegateController");
    qmlRegisterType<TripGroupController>("org.kde.itinerary", 1, 0, "TripGroupController");
    qmlRegisterType<TripGroupFilterProxyModel>("org.kde.itinerary", 1, 0, "TripGroupFilterProxyModel");
    qmlRegisterType<TripGroupProxyModel>("org.kde.itinerary", 1, 0, "TripGroupProxyModel");
    qmlRegisterType<TripGroupSplitModel>("org.kde.itinerary", 1, 0, "TripGroupSplitModel");
    qmlRegisterType<WeatherForecastModel>("org.kde.itinerary", 1, 0, "WeatherForecastModel");
#if HAVE_MATRIX
    qmlRegisterType<MatrixBeacon>("org.kde.itinerary", 1, 0, "MatrixBeacon");
#else
    qmlRegisterType<MatrixBeaconStub>("org.kde.itinerary", 1, 0, "MatrixBeacon");
#endif
    qmlRegisterType<OnlineTicketImporter>("org.kde.itinerary", 1, 0, "OnlineTicketImporter");
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
static MapDownloadManager *s_mapDownloadManager = nullptr;
static PassManager *s_passManager = nullptr;
static MatrixController *s_matrixController = nullptr;
static ImportController *s_importController = nullptr;
static TripGroupModel *s_tripGroupModel = nullptr;
static TraewellingController *s_traewellingController = nullptr;

#define REGISTER_SINGLETON_INSTANCE(Class, Instance) \
    qmlRegisterSingletonInstance<Class>("org.kde.itinerary", 1, 0, #Class, Instance);

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
    REGISTER_SINGLETON_INSTANCE(MapDownloadManager, s_mapDownloadManager)
    REGISTER_SINGLETON_INSTANCE(PassManager, s_passManager)
    REGISTER_SINGLETON_INSTANCE(MatrixController, s_matrixController);
    REGISTER_SINGLETON_INSTANCE(ImportController, s_importController);
    REGISTER_SINGLETON_INSTANCE(TripGroupModel, s_tripGroupModel);
    REGISTER_SINGLETON_INSTANCE(TraewellingController, s_traewellingController);

    REGISTER_SINGLETON_GADGET_FACTORY(DevelopmentModeController)
    REGISTER_SINGLETON_GADGET_FACTORY(Factory)
    REGISTER_SINGLETON_GADGET_FACTORY(Localizer)
    REGISTER_SINGLETON_GADGET_FACTORY(NavigationController)
    REGISTER_SINGLETON_GADGET_FACTORY(NotificationConfigController)
    REGISTER_SINGLETON_GADGET_FACTORY(PermissionManager)
    REGISTER_SINGLETON_GADGET_FACTORY(PublicTransport)
    REGISTER_SINGLETON_GADGET_FACTORY(ReservationHelper)
    REGISTER_SINGLETON_GADGET_FACTORY(UnitConversion)
    REGISTER_SINGLETON_GADGET_FACTORY(Util)

    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "About", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(KAboutData::applicationData());
    });

    qmlRegisterSingletonType<Clipboard>("org.kde.itinerary", 1, 0, "Clipboard", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new Clipboard;
    });
}

#undef REGISTER_SINGLETON_INSTANCE
#undef REGISTER_SINGLETON_GADGET_FACTORY

static QNetworkAccessManager *namFactory()
{
    static QNetworkAccessManager *s_nam = nullptr;
    if (!s_nam) {
        s_nam = new QNetworkAccessManager(QCoreApplication::instance());
        s_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        s_nam->setStrictTransportSecurityEnabled(true);
        s_nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1StringView("/hsts/"));
    }
    return s_nam;
}

void handleCommandLineArguments(ApplicationController *appController, ImportController *importController, const QStringList &args, bool isTemporary, const QString &page)
{
    for (const auto &file : args) {
        const auto localUrl = QUrl::fromLocalFile(file);
        if (QFile::exists(localUrl.toLocalFile())) {
            importController->importFromUrl(localUrl);
            if (isTemporary) {
                QFile::remove(localUrl.toLocalFile());
            }
        } else {
            const auto url = QUrl::fromUserInput(file);
            if (url.scheme() == "geo"_L1) {
                // User wants to go there
                appController->handleGeoUrl(url);
            } else {
                importController->importFromUrl(QUrl::fromUserInput(file));
            }
        }
    }

    if (!page.isEmpty()) {
        appController->requestOpenPage(page);
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
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
    KColorSchemeManager colorMgr; // enables automatic dark mode handling
#else
    QIcon::setFallbackThemeName(QStringLiteral("breeze"));
    QApplication app(argc, argv); // for native file dialogs

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif
    QGuiApplication::setApplicationDisplayName(i18n("KDE Itinerary"));
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.itinerary")));

    auto aboutData = KAboutData::applicationData();
    aboutData.setBugAddress("submit@bugs.kde.org");
    aboutData.setProductName("KDE Itinerary/general"); // Bugzilla product/component name
    aboutData.setLicense(KAboutLicense::LGPL_V2, KAboutLicense::OrLaterVersions);
    aboutData.setHomepage(QStringLiteral("https://apps.kde.org/itinerary"));
    aboutData.setShortDescription(i18n("Digital Travel Assistant"));
    aboutData.setCopyrightStatement(i18n("Copyright © The KDE Community"));
    aboutData.setDesktopFileName(QStringLiteral("org.kde.itinerary"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    QCommandLineOption isTemporaryOpt(QStringLiteral("tempfile"), QStringLiteral("Input file is a temporary file and will be deleted after importing."));
    parser.addOption(isTemporaryOpt);
    QCommandLineOption pageOpt(QStringLiteral("page"), i18nc("@info:shell", "Open Itinerary on the given page"), QStringLiteral("page"));
    parser.addOption(pageOpt);
    QCommandLineOption selfTestOpt(QStringLiteral("self-test"), QStringLiteral("internal, for automated testing"));
    parser.addOption(selfTestOpt);
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("file"), i18nc("@info:shell", "Files or URLs to import."));
    parser.process(app);
    aboutData.processCommandLine(&parser);
#if HAVE_KCRASH
    KCrash::initialize();
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_HAIKU)
    KDBusService service(KDBusService::Unique);
#endif

    IntentHandler intentHandler;

    Settings settings;
    s_settings = &settings;

    PkPassManager pkPassMgr;
    pkPassMgr.setNetworkAccessManagerFactory(namFactory);
    s_pkPassManager = &pkPassMgr;

    ReservationManager resMgr;
    s_reservationManager = &resMgr;

    DocumentManager docMgr;
    s_documentManager = &docMgr;

    FavoriteLocationModel favLocModel;
    s_favoriteLocationModel = &favLocModel;

    TripGroupManager tripGroupMgr;
    tripGroupMgr.setReservationManager(&resMgr);
    s_tripGroupManager = &tripGroupMgr;

    LiveDataManager liveDataMgr;
    liveDataMgr.setPkPassManager(&pkPassMgr);
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
    transferManager.setFavoriteLocationModel(&favLocModel);
    transferManager.setLiveDataManager(&liveDataMgr);
    transferManager.setAutoAddTransfers(settings.autoAddTransfers());
    transferManager.setAutoFillTransfers(settings.autoFillTransfers());
    QObject::connect(&settings, &Settings::autoAddTransfersChanged, &transferManager, &TransferManager::setAutoAddTransfers);
    QObject::connect(&settings, &Settings::autoFillTransfersChanged, &transferManager, &TransferManager::setAutoFillTransfers);
    s_tranferManager = &transferManager;

    tripGroupMgr.setTransferManager(&transferManager);

    TripGroupModel tripGroupModel;
    tripGroupModel.setTripGroupManager(&tripGroupMgr);
    s_tripGroupModel = &tripGroupModel;

    MapDownloadManager mapDownloadMgr;
    mapDownloadMgr.setReservationManager(&resMgr);
    mapDownloadMgr.setAutomaticDownloadEnabled(s_settings->preloadMapData());
    QObject::connect(s_settings, &Settings::preloadMapDataChanged, &mapDownloadMgr, &MapDownloadManager::setAutomaticDownloadEnabled);
    s_mapDownloadManager = &mapDownloadMgr;

    KItinerary::JsonLdDocument::registerType<GenericPkPass>();
    PassManager passMgr;
    s_passManager = &passMgr;

    MatrixController matrixController;
    s_matrixController = &matrixController;

    ImportController importController;
    importController.setNetworkAccessManagerFactory(namFactory);
    importController.setReservationManager(&resMgr);
    QObject::connect(&intentHandler, &IntentHandler::handleIntent, &importController, &ImportController::importFromIntent);
    s_importController = &importController;

    TraewellingController traewellingController(namFactory);
    s_traewellingController = &traewellingController;

    ApplicationController appController;
    appController.setNetworkAccessManagerFactory(namFactory);
    appController.setReservationManager(&resMgr);
    appController.setPkPassManager(&pkPassMgr);
    appController.setDocumentManager(&docMgr);
    appController.setFavoriteLocationModel(&favLocModel);
    appController.setTransferManager(&transferManager);
    appController.setLiveDataManager(&liveDataMgr);
    appController.setTripGroupManager(&tripGroupMgr);
    appController.setPassManager(&passMgr);
    appController.setContextTripGroupId(tripGroupModel.currentTripGroupId());
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_HAIKU)
    QObject::connect(&service, &KDBusService::activateRequested, [&](const QStringList &args, const QString &workingDir) {
        qCDebug(Log) << "remote activation" << args << workingDir;
        if (!args.isEmpty()) {
            QDir::setCurrent(workingDir);
            parser.parse(args);
            handleCommandLineArguments(&appController, &importController, parser.positionalArguments(), parser.isSet(isTemporaryOpt), parser.value(pageOpt));
        }
        if (!QGuiApplication::allWindows().isEmpty()) {
            QWindow *window = QGuiApplication::allWindows().at(0);
            KWindowSystem::updateStartupId(window);
            KWindowSystem::activateWindow(window);
        }
    });
#endif
    QObject::connect(&intentHandler, &IntentHandler::handleIntent, &appController, &ApplicationController::handleIntent);
    QObject::connect(&importController, &ImportController::infoMessage, &appController, &ApplicationController::infoMessage);

    OnlineTicketImporter::setNetworkAccessManagerFactory(namFactory);

    registerKPkPassTypes();
    registerKItineraryTypes();
    registerApplicationTypes();
    registerApplicationSingletons();

    QQmlApplicationEngine engine;

    auto pkPassImageProvider = new PkPassImageProvider;
    pkPassImageProvider->registerPassProvider([&pkPassMgr](const QString &passTypeId, const QString &serialNum) -> KPkPass::Pass* {
        return pkPassMgr.pass(passTypeId + '/'_L1 + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding)));
    });
    pkPassImageProvider->registerPassProvider([&importController](const QString &passTypeId, const QString &serialNum) -> KPkPass::Pass* {
        if (const auto it = importController.pkPasses().find(KItinerary::DocumentUtil::idForPkPass(passTypeId, serialNum)); it != importController.pkPasses().end()) {
            if (!(*it).second.pass) {
                (*it).second.pass.reset(KPkPass::Pass::fromData((*it).second.data));
            }
            return (*it).second.pass.get();
        }
        return nullptr;
    });    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), pkPassImageProvider);

    auto l10nContext = new KLocalizedContext(&engine);
    l10nContext->setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));
    engine.rootContext()->setContextObject(l10nContext);
    engine.load(QStringLiteral("qrc:/qt/qml/org/kde/itinerary/main.qml"));

    // Exit on QML load error.
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    handleCommandLineArguments(&appController, &importController, parser.positionalArguments(), parser.isSet(isTemporaryOpt), parser.value(pageOpt));

#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    intentHandler.handleIntent(Activity::getIntent());
#endif

    if (parser.isSet(selfTestOpt)) {
        QTimer::singleShot(std::chrono::milliseconds(250), &app, &QCoreApplication::quit);
    }

    return app.exec();
}

#include "main.moc"
