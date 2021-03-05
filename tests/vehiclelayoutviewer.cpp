/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransport.h"

#include <KPublicTransport/Stopover>

#include <KLocalizedContext>

#include <QQmlApplicationEngine>
#include <QQmlContext>

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

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);

    if (app.arguments().size() <= 1) {
        qCritical("Missing command line argument!");
        exit(1);
    }

    QFile file(app.arguments().at(1));
    if (!file.open(QFile::ReadOnly)) {
        qCritical("Failed to open file!");
        exit(1);
    }

    const auto stopover = KPublicTransport::Stopover::fromJson(QJsonDocument::fromJson(file.readAll()).object());

    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "PublicTransport", [](QQmlEngine*, QJSEngine *engine) -> QJSValue { \
        return engine->toScriptValue(PublicTransport()); \
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("_stopover"), stopover);
    engine.load(QStringLiteral("qrc:/vehiclelayoutviewer.qml"));

    return app.exec();
}
