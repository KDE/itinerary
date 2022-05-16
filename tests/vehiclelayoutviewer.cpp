/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransport.h"
#include "localizer.h"

#include <KPublicTransport/Stopover>

#include <KLocalizedContext>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("vehiclelayoutviewer"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption coachOpt(QStringLiteral("c"), QStringLiteral("Reserved coach number"), QStringLiteral("coach number"));
    parser.addOption(coachOpt);
    QCommandLineOption seatOpt(QStringLiteral("s"), QStringLiteral("Reserved seat number"), QStringLiteral("seat"));
    parser.addOption(seatOpt);
    parser.addPositionalArgument(QStringLiteral("stopover file"), QStringLiteral("KPT stopver JSON file"), QStringLiteral("file"));
    parser.process(app);

    if (parser.positionalArguments().empty()) {
        parser.showHelp(1);
    }

    QFile file(parser.positionalArguments().at(0));
    if (!file.open(QFile::ReadOnly)) {
        qCritical("Failed to open file!");
        exit(1);
    }

    const auto stopover = KPublicTransport::Stopover::fromJson(QJsonDocument::fromJson(file.readAll()).object());

    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PublicTransport", [](QQmlEngine*, QJSEngine *engine) -> QJSValue { \
        return engine->toScriptValue(PublicTransport()); \
    });
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "Localizer", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(Localizer());
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("_stopover"), stopover);
    engine.rootContext()->setContextProperty(QStringLiteral("_coach"), parser.value(coachOpt));
    engine.rootContext()->setContextProperty(QStringLiteral("_seat"), parser.value(seatOpt));
    engine.load(QStringLiteral("qrc:/vehiclelayoutviewer.qml"));

    return app.exec();
}
