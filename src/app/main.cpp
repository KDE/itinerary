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
#include "journeyquerymodel.h"
#include "livedatamanager.h"
#include "localizer.h"
#include "lockmanager.h"
#include "pkpassmanager.h"
#include "timelinemodel.h"
#include "pkpassimageprovider.h"
#include "publictransport.h"
#include "reservationmanager.h"
#include "settings.h"
#include "tickettokenmodel.h"
#include "tripgroupinfoprovider.h"
#include "tripgroupmanager.h"
#include "tripgroupproxymodel.h"
#include "util.h"
#include "timelinedelegatecontroller.h"
#include "weatherforecastmodel.h"

#include <weatherforecastmanager.h>

#include <KItinerary/CountryDb>
#include <KItinerary/Ticket>

#include <KPublicTransport/Manager>

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
#include "androidcontentfileengine.h"
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
    const auto activity = QtAndroid::androidActivity();
    if (!activity.isValid())
        return;

    const auto intent = activity.callObjectMethod("getIntent", "()Landroid/content/Intent;");
    appController->importFromIntent(intent);
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
    AndroidContentFileEngineHandler fsEngineHandler;
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
    resMgr.setPkPassManager(&passMgr);
    TripGroupManager tripGroupMgr;
    tripGroupMgr.setReservationManager(&resMgr);
    ApplicationController appController;
    appController.setReservationManager(&resMgr);
    appController.setPkPassManager(&passMgr);
    BrightnessManager brightnessManager;
    LockManager lockManager;

    KPublicTransport::Manager ptMgr;
    ptMgr.setAllowInsecureBackends(settings.allowInsecureServices());
    QObject::connect(&settings, &Settings::allowInsecureServicesChanged, [&ptMgr](bool insec) { ptMgr.setAllowInsecureBackends(insec); });

    LiveDataManager liveDataMgr;
    liveDataMgr.setPublicTransportManager(&ptMgr);
    liveDataMgr.setPkPassManager(&passMgr);
    liveDataMgr.setReservationManager(&resMgr);
    liveDataMgr.setPollingEnabled(settings.queryLiveData());
    QObject::connect(&settings, &Settings::queryLiveDataChanged, &liveDataMgr, &LiveDataManager::setPollingEnabled);

    JourneyQueryModel journeyQueryModel;
    journeyQueryModel.setReservationManager(&resMgr);
    journeyQueryModel.setManager(&ptMgr);

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

    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});
    qmlRegisterUncreatableType<KPkPass::Pass>("org.kde.pkpass", 1, 0, "Pass", {});
    qmlRegisterUncreatableType<KPkPass::BoardingPass>("org.kde.pkpass", 1, 0, "BoardingPass", {});

    qRegisterMetaType<KItinerary::KnowledgeDb::DrivingSide>();
    qmlRegisterUncreatableType<KItinerary::Ticket>("org.kde.kitinerary", 1, 0, "Ticket", {});
    qmlRegisterUncreatableMetaObject(KItinerary::KnowledgeDb::staticMetaObject, "org.kde.kitinerary", 1, 0, "KnowledgeDb", {});

    qmlRegisterUncreatableType<CountryInformation>("org.kde.itinerary", 1, 0, "CountryInformation", {});
    qmlRegisterType<CountryModel>("org.kde.itinerary", 1, 0, "CountryModel");
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "Localizer", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(Localizer());
    });
    qmlRegisterType<TicketTokenModel>("org.kde.itinerary", 1, 0, "TicketTokenModel");
    qmlRegisterUncreatableType<TimelineModel>("org.kde.itinerary", 1, 0, "TimelineModel", {});
    qmlRegisterType<TimelineDelegateController>("org.kde.itinerary", 1, 0, "TimelineDelegateController");
    qmlRegisterSingletonType<Util>("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine*, QJSEngine*) -> QObject*{
        return new Util;
    });
    qmlRegisterType<WeatherForecastModel>("org.kde.itinerary", 1, 0, "WeatherForecastModel");
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PublicTransport", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(PublicTransportUtil());
    });

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("_pkpassManager"), &passMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_reservationManager"), &resMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_timelineModel"), &tripGroupProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("_appController"), &appController);
    engine.rootContext()->setContextProperty(QStringLiteral("_settings"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("_weatherForecastManager"), &weatherForecastMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_brightnessManager"), &brightnessManager);
    engine.rootContext()->setContextProperty(QStringLiteral("_lockManager"), &lockManager);
    engine.rootContext()->setContextProperty(QStringLiteral("_liveDataManager"), &liveDataMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_journeyQueryModel"), &journeyQueryModel);
    engine.rootContext()->setContextProperty(QStringLiteral("_tripGroupInfoProvider"), QVariant::fromValue(tripGroupInfoProvider));
    engine.load(QStringLiteral("qrc:/main.qml"));

    handlePositionalArguments(&appController, parser.positionalArguments());
    handleViewIntent(&appController);

    return app.exec();
}
