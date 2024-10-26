/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "journeysectionmodel.h"
#include "localizer.h"
#include "publictransport.h"
#include "util.h"

#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Location>

#include <KLocalizedContext>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QUrl>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("journeyquerytest"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption backendOpt(QStringLiteral("b"), QStringLiteral("KPT backend"), QStringLiteral("backend id"));
    parser.addOption(backendOpt);
    QCommandLineOption fromOpt(QStringLiteral("f"), QStringLiteral("Origin station name"), QStringLiteral("from"));
    parser.addOption(fromOpt);
    QCommandLineOption toOpt(QStringLiteral("t"), QStringLiteral("Destination station name"), QStringLiteral("to"));
    parser.addOption(toOpt);
    parser.process(app);

    if (!parser.isSet(fromOpt) || !parser.isSet(backendOpt) || !parser.isSet(backendOpt)) {
        parser.showHelp(1);
    }

    KPublicTransport::JourneyRequest req;
    req.setBackendIds({parser.value(backendOpt)});
    KPublicTransport::Location from;
    from.setName(parser.value(fromOpt));
    req.setFrom(from);
    KPublicTransport::Location to;
    to.setName(parser.value(toOpt));
    req.setTo(to);
    req.setIncludeIntermediateStops(true);

    qmlRegisterType<JourneySectionModel>("org.kde.itinerary", 1, 0, "JourneySectionModel");
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "Localizer", [](QQmlEngine *, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(Localizer());
    });
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PublicTransport", [](QQmlEngine *, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(PublicTransport());
    });
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine *, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(Util());
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("_request"), req);
    engine.load(QStringLiteral("qrc:/qt/qml/org/kde/itinerary/journeyquerytest.qml"));

    return app.exec();
}
