/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "itinerary_version.h"
#include "logging.h"

#include "applicationcontroller.h"
#include "clipboard.h"
#include "countrysubdivisionmodel.h"
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
#include "migrator.h"
#include "navigationcontroller.h"
#include "notificationconfigcontroller.h"
#include "onlineticketimporter.h"
#include "passmanager.h"
#include "permissionmanager.h"
#include "pkpassimageprovider.h"
#include "pkpassmanager.h"
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
#include "transferdelegatecontroller.h"
#include "transfermanager.h"
#include "traewellingcontroller.h"
#include "tripgroupcontroller.h"
#include "tripgroupfilterproxymodel.h"
#include "tripgrouplocationmodel.h"
#include "tripgroupmanager.h"
#include "tripgroupmapmodel.h"
#include "tripgroupmodel.h"
#include "tripgroupsplitmodel.h"
#include "unitconversion.h"
#include "util.h"
#include "weatherforecastmodel.h"

#if HAVE_MATRIX
#include "matrix/matrixbeacon.h"
#include "matrix/matrixroomsmodel.h"
#endif

#include "weatherforecastmanager.h"

#include <KItinerary/CountryDb>
#include <KItinerary/DocumentUtil>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PriceUtil>
#include <KItinerary/Ticket>

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Field>

#ifndef Q_OS_ANDROID
#include <KDBusService>
#include <KWindowSystem>
#else
#include <KColorSchemeManager>
#endif

#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <KAboutData>
#if HAVE_KCRASH
#include <KCrash>
#endif

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

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

void registerKItineraryTypes()
{
    qRegisterMetaType<KItinerary::KnowledgeDb::DrivingSide>();
    qmlRegisterUncreatableMetaObject(KItinerary::Ticket::staticMetaObject, "org.kde.kitinerary", 1, 0, "Ticket", {});
    qmlRegisterUncreatableMetaObject(KItinerary::KnowledgeDb::staticMetaObject, "org.kde.kitinerary", 1, 0, "KnowledgeDb", {});
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PriceUtil", [](QQmlEngine *, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(KItinerary::PriceUtil());
    });
}

#define REGISTER_SINGLETON_INSTANCE(Class, Instance) qmlRegisterSingletonInstance<Class>("org.kde.itinerary", 1, 0, #Class, Instance);

#define REGISTER_SINGLETON_GADGET_FACTORY(Class)                                                                                                               \
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, #Class, [](QQmlEngine *, QJSEngine *engine) -> QJSValue {                                              \
        return engine->toScriptValue(Class());                                                                                                                 \
    });

void registerApplicationSingletons()
{
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

void handleCommandLineArguments(ApplicationController *appController,
                                ImportController *importController,
                                const QStringList &args,
                                bool isTemporary,
                                const QString &page)
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
    KColorSchemeManager::instance(); // enables automatic dark mode handling
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

    Migrator::run();

    QQmlApplicationEngine engine;

    // Settings
    const auto settings = engine.singletonInstance<Settings *>("org.kde.itinerary", "Settings");
    Q_ASSERT(settings);

    // PkPassManager
    const auto pkPassManager = engine.singletonInstance<PkPassManager *>("org.kde.itinerary", "PkPassManager");
    Q_ASSERT(pkPassManager);
    pkPassManager->setNetworkAccessManagerFactory(namFactory);

    // ReservationManager
    const auto reservationManager = engine.singletonInstance<ReservationManager *>("org.kde.itinerary", "ReservationManager");
    Q_ASSERT(reservationManager);

    // DocumentManager
    const auto documentManager = engine.singletonInstance<DocumentManager *>("org.kde.itinerary", "DocumentManager");
    Q_ASSERT(documentManager);

    // TripGroupManager
    const auto tripGroupManager = engine.singletonInstance<TripGroupManager *>("org.kde.itinerary", "TripGroupManager");
    Q_ASSERT(tripGroupManager);
    tripGroupManager->setReservationManager(reservationManager);

    // FavoriteLocationModel
    const auto favoriteLocationModel = engine.singletonInstance<FavoriteLocationModel *>("org.kde.itinerary", "FavoriteLocationModel");
    Q_ASSERT(favoriteLocationModel);

    // LiveDataManager
    const auto liveDataManager = engine.singletonInstance<LiveDataManager *>("org.kde.itinerary", "LiveDataManager");
    Q_ASSERT(liveDataManager);
    liveDataManager->setPkPassManager(pkPassManager);
    liveDataManager->setReservationManager(reservationManager);
    liveDataManager->setPollingEnabled(settings->queryLiveData());
    liveDataManager->setShowNotificationsOnLockScreen(settings->showNotificationOnLockScreen());
    QObject::connect(settings, &Settings::queryLiveDataChanged, liveDataManager, &LiveDataManager::setPollingEnabled);
    QObject::connect(settings, &Settings::showNotificationOnLockScreenChanged, liveDataManager, &LiveDataManager::setShowNotificationsOnLockScreen);

    // WeatherForecastManager
    const auto weatherForecastManager = engine.singletonInstance<WeatherForecastManager *>("org.kde.itinerary", "WeatherForecastManager");
    Q_ASSERT(weatherForecastManager);
    weatherForecastManager->setAllowNetworkAccess(settings->weatherForecastEnabled());
    QObject::connect(settings, &Settings::weatherForecastEnabledChanged, weatherForecastManager, &WeatherForecastManager::setAllowNetworkAccess);

    // TransferManager
    const auto transferManager = engine.singletonInstance<TransferManager *>("org.kde.itinerary", "TransferManager");
    Q_ASSERT(transferManager);
    transferManager->setReservationManager(reservationManager);
    transferManager->setFavoriteLocationModel(favoriteLocationModel);
    transferManager->setLiveDataManager(liveDataManager);
    transferManager->setAutoAddTransfers(settings->autoAddTransfers());
    transferManager->setAutoFillTransfers(settings->autoFillTransfers());
    QObject::connect(settings, &Settings::autoAddTransfersChanged, transferManager, &TransferManager::setAutoAddTransfers);
    QObject::connect(settings, &Settings::autoFillTransfersChanged, transferManager, &TransferManager::setAutoFillTransfers);

    tripGroupManager->setTransferManager(transferManager);

    // TripGroupModel
    const auto tripGroupModel = engine.singletonInstance<TripGroupModel *>("org.kde.itinerary", "TripGroupModel");
    Q_ASSERT(tripGroupModel);
    tripGroupModel->setTripGroupManager(tripGroupManager);

    // MapDownloadManager
    const auto mapDownloadManager = engine.singletonInstance<MapDownloadManager *>("org.kde.itinerary", "MapDownloadManager");
    Q_ASSERT(mapDownloadManager);
    mapDownloadManager->setReservationManager(reservationManager);
    mapDownloadManager->setAutomaticDownloadEnabled(settings->preloadMapData());
    QObject::connect(settings, &Settings::preloadMapDataChanged, mapDownloadManager, &MapDownloadManager::setAutomaticDownloadEnabled);

    KItinerary::JsonLdDocument::registerType<GenericPkPass>();

    const auto passManager = engine.singletonInstance<PassManager *>("org.kde.itinerary", "PassManager");
    Q_ASSERT(passManager);

    IntentHandler intentHandler;

    // ImportController
    const auto importController = engine.singletonInstance<ImportController *>("org.kde.itinerary", "ImportController");
    Q_ASSERT(importController);
    importController->setNetworkAccessManagerFactory(namFactory);
    importController->setReservationManager(reservationManager);
    QObject::connect(&intentHandler, &IntentHandler::handleIntent, importController, &ImportController::importFromIntent);

    // ApplicationController
    const auto applicationController = engine.singletonInstance<ApplicationController *>("org.kde.itinerary", "ApplicationController");
    Q_ASSERT(applicationController);
    applicationController->setNetworkAccessManagerFactory(namFactory);
    applicationController->setReservationManager(reservationManager);
    applicationController->setPkPassManager(pkPassManager);
    applicationController->setDocumentManager(documentManager);
    applicationController->setFavoriteLocationModel(favoriteLocationModel);
    applicationController->setTransferManager(transferManager);
    applicationController->setLiveDataManager(liveDataManager);
    applicationController->setTripGroupManager(tripGroupManager);
    applicationController->setPassManager(passManager);
    applicationController->setContextTripGroupId(tripGroupModel->currentTripGroupId());
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_HAIKU)
    QObject::connect(&service, &KDBusService::activateRequested, [&](const QStringList &args, const QString &workingDir) {
        qCDebug(Log) << "remote activation" << args << workingDir;
        if (!args.isEmpty()) {
            QDir::setCurrent(workingDir);
            parser.parse(args);
            handleCommandLineArguments(applicationController, importController, parser.positionalArguments(), parser.isSet(isTemporaryOpt), parser.value(pageOpt));
        }
        if (!QGuiApplication::allWindows().isEmpty()) {
            QWindow *window = QGuiApplication::allWindows().at(0);
            KWindowSystem::updateStartupId(window);
            KWindowSystem::activateWindow(window);
        }
    });
#endif
    QObject::connect(&intentHandler, &IntentHandler::handleIntent, applicationController, &ApplicationController::handleIntent);
    QObject::connect(importController, &ImportController::infoMessage, applicationController, &ApplicationController::infoMessage);
    QObject::connect(applicationController, &ApplicationController::reloadSettings, settings, &Settings::reloadSettings);

    const auto traewellingController = engine.singletonInstance<TraewellingController *>("org.kde.itinerary", "TraewellingController");
    Q_ASSERT(traewellingController);
    traewellingController->setNamFactory(namFactory);
 
    OnlineTicketImporter::setNetworkAccessManagerFactory(namFactory);

    registerKItineraryTypes();
    registerApplicationSingletons();

    auto pkPassImageProvider = new PkPassImageProvider;
    pkPassImageProvider->registerPassProvider([pkPassManager](const QString &passTypeId, const QString &serialNum) -> KPkPass::Pass * {
        return pkPassManager->pass(passTypeId + '/'_L1 + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding)));
    });
    pkPassImageProvider->registerPassProvider([importController](const QString &passTypeId, const QString &serialNum) -> KPkPass::Pass * {
        if (const auto it = importController->pkPasses().find(KItinerary::DocumentUtil::idForPkPass(passTypeId, serialNum));
            it != importController->pkPasses().end()) {
            if (!(*it).second.pass) {
                (*it).second.pass.reset(KPkPass::Pass::fromData((*it).second.data));
            }
            return (*it).second.pass.get();
        }
        return nullptr;
    });
    engine.addImageProvider(u"org.kde.pkpass"_s, pkPassImageProvider);

    KLocalization::setupLocalizedContext(&engine);
    engine.loadFromModule("org.kde.itinerary", "Main");

    // Exit on QML load error.
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    handleCommandLineArguments(applicationController, importController, parser.positionalArguments(), parser.isSet(isTemporaryOpt), parser.value(pageOpt));

#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    intentHandler.handleIntent(Activity::getIntent());
#endif

    if (parser.isSet(selfTestOpt)) {
        QTimer::singleShot(std::chrono::milliseconds(250), &app, &QCoreApplication::quit);
    }

    return app.exec();
}
