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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "itinerary_version.h"
#include "logging.h"

#include "applicationcontroller.h"
#include "countryinformation.h"
#include "countrymodel.h"
#include "localizer.h"
#include "pkpassmanager.h"
#include "timelinemodel.h"
#include "pkpassimageprovider.h"
#include "reservationmanager.h"
#include "settings.h"

#include <weatherforecastmanager.h>

#include <KItinerary/CountryDb>
#include <KItinerary/Ticket>

#include <KPkPass/Field>
#include <KPkPass/Barcode>

#include <KLocalizedContext>
#include <KLocalizedString>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#endif

#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>

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


#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kde-itinerary"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral(ITINERARY_VERSION_STRING));

    QGuiApplication::setApplicationDisplayName(i18n("KDE Itinerary"));
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("map-globe")));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"), i18n("PkPass or JSON-LD file to import."));
    parser.process(app);

    Settings settings;
    PkPassManager passMgr;
    ReservationManager resMgr;
    resMgr.setPkPassManager(&passMgr);
    ApplicationController appController;
    appController.setReservationManager(&resMgr);
    appController.setPkPassManager(&passMgr);

    TimelineModel timelineModel;
    timelineModel.setHomeCountryIsoCode(settings.homeCountryIsoCode());
    timelineModel.setReservationManager(&resMgr);
    QObject::connect(&settings, &Settings::homeCountryIsoCodeChanged, &timelineModel, &TimelineModel::setHomeCountryIsoCode);

    WeatherForecastManager weatherForecastMgr;
    weatherForecastMgr.setAllowNetworkAccess(settings.weatherForecastEnabled());
    QObject::connect(&settings, &Settings::weatherForecastEnabledChanged, &weatherForecastMgr, &WeatherForecastManager::setAllowNetworkAccess);
    timelineModel.setWeatherForecastManager(&weatherForecastMgr);


    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});

    qRegisterMetaType<KItinerary::KnowledgeDb::DrivingSide>();
    qmlRegisterUncreatableType<KItinerary::Ticket>("org.kde.kitinerary", 1, 0, "Ticket", {});
    qmlRegisterUncreatableMetaObject(KItinerary::KnowledgeDb::staticMetaObject, "org.kde.kitinerary", 1, 0, "KnowledgeDb", {});

    qmlRegisterUncreatableType<CountryInformation>("org.kde.itinerary", 1, 0, "CountryInformation", {});
    qmlRegisterUncreatableType<TimelineModel>("org.kde.itinerary", 1, 0, "TimelineModel", {});
    qmlRegisterSingletonType<Localizer>("org.kde.itinerary", 1, 0, "Localizer", [](QQmlEngine*, QJSEngine*) -> QObject*{
        return new Localizer;
    });
    qmlRegisterType<CountryModel>("org.kde.itinerary", 1, 0, "CountryModel");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("_pkpassManager"), &passMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_reservationManager"), &resMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_timelineModel"), &timelineModel);
    engine.rootContext()->setContextProperty(QStringLiteral("_appController"), &appController);
    engine.rootContext()->setContextProperty(QStringLiteral("_settings"), &settings);
    engine.load(QStringLiteral(":/main.qml"));

    for (const auto &file : parser.positionalArguments()) {
        if (file.endsWith(QLatin1String(".pkpass")))
            passMgr.importPass(QUrl::fromLocalFile(file));
        else
            resMgr.importReservation(QUrl::fromLocalFile(file));
    }
    handleViewIntent(&appController);

    return app.exec();
}
