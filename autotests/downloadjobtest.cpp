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
             QByteArray("<osm version=\"0.6\"><node id=\"448814556\" lat=\"52.5135789\" lon=\"13.4184810\"><tag k=\"addr:city\" v=\"Berlin\"/><tag "
                        "k=\"addr:country\" v=\"DE\"/><tag k=\"addr:housenumber\" v=\"6\"/><tag k=\"addr:postcode\" v=\"10179\"/><tag k=\"addr:street\" "
                        "v=\"Brückenstraße\"/><tag k=\"addr:suburb\" v=\"Mitte\"/><tag k=\"amenity\" v=\"restaurant\"/><tag k=\"contact:website\" "
                        "v=\"http://www.ming-dynastie.de\"/><tag k=\"cuisine\" v=\"chinese\"/><tag k=\"name\" v=\"Ming Dynastie\"/></node></osm>"),
             QString()});

        DownloadJob job(QUrl(QStringLiteral("https://www.openstreetmap.org/node/448814556")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(QStringLiteral("https://www.openstreetmap.org/api/0.6/node/448814556")));

        QCOMPARE(job.hasError(), false);
        QCOMPARE(job.errorMessage(), QString());
        QVERIFY(job.data().contains("@context"));
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
