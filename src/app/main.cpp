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
#include "pkpassmanager.h"
#include "timelinemodel.h"
#include "pkpassimageprovider.h"
#include "reservationmanager.h"

#include <KPkPass/Field>
#include <KPkPass/Barcode>

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

void handleViewIntent(PkPassManager *passMgr)
{
#ifdef Q_OS_ANDROID
    // handle opened files
    const auto activity = QtAndroid::androidActivity();
    if (!activity.isValid())
        return;

    const auto intent = activity.callObjectMethod("getIntent", "()Landroid/content/Intent;");
    if (!intent.isValid())
        return;

    const auto uri = intent.callObjectMethod("getData", "()Landroid/net/Uri;");
    if (!uri.isValid())
        return;

    const auto scheme = uri.callObjectMethod("getScheme", "()Ljava/lang/String;");
    if (scheme.toString() == QLatin1String("content")) {
        const auto tmpFile = activity.callObjectMethod("receiveContent", "(Landroid/net/Uri;)Ljava/lang/String;", uri.object<jobject>());
        passMgr->importPassFromTempFile(tmpFile.toString());
    } else if (scheme.toString() == QLatin1String("file")) {
        const auto uriStr = uri.callObjectMethod("toString", "()Ljava/lang/String;");
        passMgr->importPass(QUrl(uriStr.toString()));
    } else {
        const auto uriStr = uri.callObjectMethod("toString", "()Ljava/lang/String;");
        qCWarning(Log) << "Unknown intent URI:" << uriStr.toString();
    }
#else
    Q_UNUSED(passMgr);
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

    QGuiApplication::setApplicationDisplayName(QStringLiteral("KDE Itinerary")); // TODO i18n
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("map-globe")));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("pass", QStringLiteral("PkPass file to import."));
    parser.process(app);

    PkPassManager passMgr;
    ReservationManager resMgr;
    resMgr.setPkPassManager(&passMgr);
    TimelineModel timelineModel;
    timelineModel.setPkPassManager(&passMgr);
    timelineModel.setReservationManager(&resMgr);

    ApplicationController appController;

    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});

    qmlRegisterUncreatableType<TimelineModel>("org.kde.itinerary", 1, 0, "TimelineModel", {});

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextProperty(QStringLiteral("_pkpassManager"), &passMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_reservationManager"), &resMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_timelineModel"), &timelineModel);
    engine.rootContext()->setContextProperty(QStringLiteral("_appController"), &appController);
    engine.load(QStringLiteral(":/main.qml"));

    for (const auto &file : parser.positionalArguments()) {
        if (file.endsWith(QLatin1String(".pkpass")))
            passMgr.importPass(QUrl::fromLocalFile(file));
        else
            resMgr.importReservation(QUrl::fromLocalFile(file));
    }
    handleViewIntent(&passMgr);

    return app.exec();
}
