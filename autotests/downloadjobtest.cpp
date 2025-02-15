/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mocknetworkaccessmanager.h"
#include "testhelper.h"

#include "downloadjob.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class DownloadJobTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testRegularDownload()
    {
        m_nam.requests.clear();
        m_nam.replies.push({QNetworkReply::ContentNotFoundError, 404, QByteArray(), QString()});
        m_nam.replies.push({QNetworkReply::NoError, 200, QByteArray("<html><body>Hello</body></html>"), QString()});

        DownloadJob job(QUrl(QStringLiteral("https://akademy.kde.org/2023/")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 2); // (failed) activity pub request and the actual download
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
        QVERIFY(m_nam.requests[0].request.rawHeader("Accept").contains("activitystreams"));

        QCOMPARE(m_nam.requests[1].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[1].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
        QVERIFY(!m_nam.requests[1].request.rawHeader("Accept").contains("activitystreams"));

        QCOMPARE(job.hasError(), false);
        QCOMPARE(job.errorMessage(), QString());
        QVERIFY(job.data().contains("Hello"));
    }

    void testOnlineTicketDownload()
    {
        m_nam.requests.clear();
        DownloadJob job(QUrl(QStringLiteral("http://dbnavigator.bahn.de/loadorder?&on=ABC123&name=Konqi&a.deeplink.id=&a.launch.campaign.trackingcode=ABC")),
                        &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::PostOperation);
        QCOMPARE(m_nam.requests[0].data,
                 QByteArray("<rqorderdetails version=\"1.0\"><rqheader v=\"23040000\" os=\"KCI\" app=\"KCI-Webservice\"/><rqorder on=\"ABC123\"/><authname "
                            "tln=\"KONQI\"/></rqorderdetails>"));
    }

    void testActivityPubDownload()
    {
        m_nam.requests.clear();
        m_nam.replies.push({QNetworkReply::NoError, 200, QByteArray("{\"@context\": \"https://www.w3.org/ns/activitystreams\"}"), QString()});

        DownloadJob job(QUrl(QStringLiteral("https://akademy.kde.org/2023/")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
        QVERIFY(m_nam.requests[0].request.rawHeader("Accept").contains("activitystreams"));

        QCOMPARE(job.hasError(), false);
        QCOMPARE(job.errorMessage(), QString());
        QVERIFY(job.data().contains("@context"));
    }

    void testOsmNode()
    {
        m_nam.requests.clear();
        m_nam.replies.push(
            {QNetworkReply::NoError,
             200,
             QByteArray("{\"type\":\"FeatureCollection\",\"geocoding\":{\"version\":\"0.1.0\",\"attribution\":\"Data © OpenStreetMap contributors, ODbL 1.0. http://osm.org/copyright\",\"licence\":\"ODbL\"},\"features\":[{\"type\":\"Feature\",\"properties\":{\"geocoding\":{\"place_id\":133218569,\"osm_type\":\"node\",\"osm_id\":448814556,\"osm_key\":\"amenity\",\"osm_value\":\"restaurant\",\"type\":\"house\",\"label\":\"Ming Dynastie, 6, Brückenstraße, Luisenstadt, Mitte, Berlin, 10179, Germany\",\"name\":\"Ming Dynastie\",\"housenumber\":\"6\",\"postcode\":\"10179\",\"street\":\"Brückenstraße\",\"locality\":\"Luisenstadt\",\"district\":\"Mitte\",\"city\":\"Berlin\",\"country\":\"Germany\",\"country_code\":\"de\",\"admin\":{\"level10\":\"Mitte\",\"level9\":\"Mitte\",\"level4\":\"Berlin\"}}},\"geometry\":{\"type\": \"Point\",\"coordinates\": [13.418481, 52.5135789]}}]}"),
             QString()});

        DownloadJob job(QUrl(QStringLiteral("https://www.openstreetmap.org/node/448814556")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(u"https://nominatim.openstreetmap.org/lookup?osm_ids=N448814556&extratags=1&format=geocodejson"_s));

        QCOMPARE(job.hasError(), false);
        QCOMPARE(job.errorMessage(), QString());
        QVERIFY(job.data().contains("@context"));
        QVERIFY(job.data().contains("FoodEstablishment"));
    }

    void test404()
    {
        m_nam.requests.clear();
        m_nam.replies.push({QNetworkReply::ContentNotFoundError, 404, QByteArray("garbage"), QStringLiteral("Invalid Request")});
        m_nam.replies.push({QNetworkReply::ContentNotFoundError, 404, QByteArray("garbage"), QStringLiteral("File not found")});

        DownloadJob job(QUrl(QStringLiteral("https://akademy.kde.org/2023/")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 2); // (failed) activity pub request and the actual download
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
        QVERIFY(m_nam.requests[0].request.rawHeader("Accept").contains("activitystreams"));

        QCOMPARE(m_nam.requests[1].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[1].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
        QVERIFY(!m_nam.requests[1].request.rawHeader("Accept").contains("activitystreams"));

        QCOMPARE(job.hasError(), true);
        QVERIFY(job.errorMessage().contains(QLatin1StringView("File not found")));
        QCOMPARE(job.data(), QByteArray());
    }

    void testNetworkFailure()
    {
        m_nam.requests.clear();

        DownloadJob job(QUrl(QStringLiteral("https://akademy.kde.org/2023/")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
        QVERIFY(job.hasError());
        QVERIFY(!job.errorMessage().isEmpty());
        QCOMPARE(job.data(), QByteArray());
    }

private:
    MockNetworkAccessManager m_nam;
};

QTEST_GUILESS_MAIN(DownloadJobTest)

#include "downloadjobtest.moc"
