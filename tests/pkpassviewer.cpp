/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassmanager.h"
#include "pkpassimageprovider.h"
#include "util.h"

#include <KPkPass/Field>
#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>

#include <KLocalizedContext>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDebug>
#include <QGuiApplication>
#include <QUrl>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("pkpassviewer"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);

    if (app.arguments().size() <= 1) {
        qCritical("Missing command line argument!");
        exit(1);
    }

    PkPassManager passMgr;
    const auto passId = passMgr.importPass(QUrl::fromLocalFile(app.arguments().at(1)));
    if (passId.isEmpty()) {
        qCritical("Failed to open or parse pkpass file!");
        exit(1);
    }

    qmlRegisterUncreatableType<KPkPass::Barcode>("org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableType<KPkPass::Field>("org.kde.pkpass", 1, 0, "Field", {});
    qmlRegisterUncreatableType<KPkPass::Pass>("org.kde.pkpass", 1, 0, "Pass", {});
    qmlRegisterUncreatableType<KPkPass::BoardingPass>("org.kde.pkpass", 1, 0, "BoardingPass", {});
    qmlRegisterSingletonType<Util>("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine*, QJSEngine*) -> QObject*{
        return new Util;
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextProperty(QStringLiteral("_passId"), passId);
    engine.rootContext()->setContextProperty(QStringLiteral("_pass"), passMgr.pass(passId));
    engine.load(QStringLiteral("qrc:/pkpassviewer.qml"));

    return app.exec();
}

