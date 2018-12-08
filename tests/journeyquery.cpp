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
#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Line>
#include <KPublicTransport/Location>
#include <KPublicTransport/Manager>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QUrl>


using namespace KPublicTransport;

class QueryManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
public:
    QueryManager()
    {
        ptMgr.setNetworkAccessManager(&nam);
    }

    Q_INVOKABLE void findJourney(double fromLat, double fromLon, double toLat, double toLon)
    {
        engine->rootContext()->setContextProperty(QStringLiteral("_journeys"), QVariantList());
        m_loading = true;
        emit loadingChanged();
        m_errorMsg.clear();
        emit errorMessageChanged();

        Location from;
        from.setCoordinate(fromLat, fromLon);
        Location to;
        to.setCoordinate(toLat, toLon);

        auto reply = ptMgr.findJourney({from, to});
        QObject::connect(reply, &JourneyReply::finished, [reply, this]{
            m_loading = false;
            emit loadingChanged();

            if (reply->error() == JourneyReply::NoError) {
                const auto res = reply->journeys();
                QVariantList l;
                l.reserve(res.size());
                std::transform(res.begin(), res.end(), std::back_inserter(l), [](const auto &journey) { return QVariant::fromValue(journey); });
                engine->rootContext()->setContextProperty(QStringLiteral("_journeys"), l);

                for (const auto &journey : res) {
                    qDebug() << journey.sections().size();
                    for (const auto &section : journey.sections()) {
                        qDebug() << " From" << section.from().name() << section.departureTime();
                        qDebug() << " Mode" << section.mode() << section.route().line().name() << section.route().direction() << section.route().line().modeString();
                        qDebug() << " To" << section.to().name() << section.arrivalTime();
                    }
                }
            } else {
                m_errorMsg = reply->errorString();
                emit errorMessageChanged();
            }
        });
    }

    bool loading() const { return m_loading; }
    QString errorMessage() const { return m_errorMsg; }

    QQmlEngine *engine = nullptr;

signals:
    void loadingChanged();
    void errorMessageChanged();

private:
    QNetworkAccessManager nam;
    Manager ptMgr;
    QString m_errorMsg;
    bool m_loading = false;
};

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

    QueryManager mgr;
    QQmlApplicationEngine engine;
    mgr.engine = &engine;
    engine.rootContext()->setContextProperty(QStringLiteral("_queryMgr"), &mgr);
    engine.load(QStringLiteral("qrc:/journeyquery.qml"));
    return app.exec();
}

#include "journeyquery.moc"
