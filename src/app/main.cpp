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

#include "pkpassmanager.h"
#include "timelinemodel.h"
#include "pkpassimageprovider.h"

#include <KPkPass/Field>
#include <KPkPass/Barcode>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#endif

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>

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
    TimelineModel timelineModel;
    timelineModel.setPkPassManager(&passMgr);

    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextProperty(QStringLiteral("_pkpassManager"), &passMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("_timelineModel"), &timelineModel);
    engine.load(QStringLiteral(":/main.qml"));

    for (const auto &file : parser.positionalArguments())
        passMgr.importPass(QUrl::fromLocalFile(file));

#ifdef Q_OS_ANDROID
    // handle opened files
    const auto activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        const auto intent = activity.callObjectMethod("getIntent", "()Landroid/content/Intent;");
        if (intent.isValid()) {
            const auto data = intent.callObjectMethod("getDataString", "()Ljava/lang/String;");
            if (data.isValid()) {
                // TODO handle content:// urls
                passMgr.importPass(QUrl(data.toString()));
            }
        }
    }
#endif

    return app.exec();
}
