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

#include "pkpassmanager.h"
#include "pkpassimageprovider.h"

#include <KPkPass/Field>
#include <KPkPass/Barcode>
#include <KPkPass/Pass>

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

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("org.kde.pkpass"), new PkPassImageProvider(&passMgr));
    engine.rootContext()->setContextProperty(QStringLiteral("_passId"), passId);
    engine.rootContext()->setContextProperty(QStringLiteral("_pass"), passMgr.pass(passId));
    engine.load(QStringLiteral("qrc:/pkpassviewer.qml"));

    return app.exec();
}

