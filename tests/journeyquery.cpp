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

#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>
#include <KPublicTransport/Location>
#include <KPublicTransport/NavitiaClient>
#include <KPublicTransport/NavitiaParser>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>


using namespace KPublicTransport;

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("journeyquery"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);

    qmlRegisterUncreatableType<KPublicTransport::Line>("org.kde.kpublictransport", 1, 0, "Line", {});
    qmlRegisterUncreatableType<KPublicTransport::JourneySection>("org.kde.kpublictransport", 1, 0, "JourneySection", {});

    QQmlApplicationEngine engine;
    engine.load(QStringLiteral("qrc:/journeyquery.qml"));

    QNetworkAccessManager nam;

    Location from;
    from.setCoordinate(2.57110, 49.00406);
    Location to;
    to.setCoordinate(2.37708, 48.84388);
    auto reply = NavitiaClient::findJourney(from, to, QDateTime::currentDateTime(), &nam);
    QObject::connect(reply, &QNetworkReply::finished, [reply, &app, &engine]{
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            app.exit();
        } else {
            qDebug() << "Success!";
            auto res = NavitiaParser::parseJourneys(reply->readAll());
            QVariantList l;
            l.reserve(res.size());
            std::transform(res.begin(), res.end(), std::back_inserter(l), [](const auto &journey) { return QVariant::fromValue(journey); });
            engine.rootContext()->setContextProperty(QStringLiteral("_journeys"), l);

            for (const auto &journey : res) {
                qDebug() << journey.sections().size();
                for (const auto &section : journey.sections()) {
                    qDebug() << " From" << section.from().name() << section.departureTime();
                    qDebug() << " Mode" << section.mode() << section.route().line().name() << section.route().direction() << section.route().line().modeString();
                    qDebug() << " To" << section.to().name() << section.arrivalTime();
                }
            }
        }
    });

    return app.exec();
}
