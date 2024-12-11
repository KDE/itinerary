/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassimageprovider.h"
#include "pkpassmanager.h"
#include "util.h"

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Field>

#include <KLocalizedQmlContext>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDebug>
#include <QGuiApplication>
#include <QUrl>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("pkpassviewer"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
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

    qmlRegisterUncreatableMetaObject(KPkPass::Barcode::staticMetaObject, "org.kde.pkpass", 1, 0, "Barcode", {});
    qmlRegisterUncreatableMetaObject(KPkPass::Field::staticMetaObject, "org.kde.pkpass", 1, 0, "Field", {});
    qmlRegisterUncreatableType<KPkPass::Pass>("org.kde.pkpass", 1, 0, "Pass", {});
    qmlRegisterUncreatableType<KPkPass::BoardingPass>("org.kde.pkpass", 1, 0, "BoardingPass", {});
    qmlRegisterSingletonType("org.kde.itinerary", 1, 0, "Util", [](QQmlEngine *, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(Util());
    });

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);

    auto pkPassImageProvider = new PkPassImageProvider;
    pkPassImageProvider->registerPassProvider([&passMgr](const QString &passTypeId, const QString &serialNum) -> KPkPass::Pass * {
        return passMgr.pass(passTypeId + '/'_L1 + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding)));
    });
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), pkPassImageProvider);

    engine.rootContext()->setContextProperty(QStringLiteral("_passId"), passId);
    engine.rootContext()->setContextProperty(QStringLiteral("_pass"), passMgr.pass(passId));
    engine.load(QStringLiteral("qrc:/qt/qml/org/kde/itinerary/pkpassviewer.qml"));

    return app.exec();
}
