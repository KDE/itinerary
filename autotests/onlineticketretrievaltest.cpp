/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"
#include "mocknetworkaccessmanager.h"

#include <onlineticketretrievaljob.h>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class OnlineTicketRetrievalTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testTicketRetrieval_data()
    {
        QTest::addColumn<QString>("sourceId");
        QTest::addColumn<QVariantMap>("arguments");
        QTest::addColumn<QByteArray>("postData");

        QTest::newRow("db") << QStringLiteral("db") << QVariantMap({ {QStringLiteral("name"), QStringLiteral("KONQUI")}, {QStringLiteral("reference"), QStringLiteral("XYZ007")}})
            << QByteArray(R"(<rqorderdetails version="1.0"><rqheader v="23040000" os="KCI" app="KCI-Webservice"/><rqorder on="XYZ007"/><authname tln="KONQUI"/></rqorderdetails>)");
        QTest::newRow("sncf") << QStringLiteral("sncf") << QVariantMap({ {QStringLiteral("name"), QStringLiteral("KONQUI")}, {QStringLiteral("reference"), QStringLiteral("XYZ007")}})
            << QByteArray(R"({"reference":"XYZ007","name":"KONQUI"})");
    }

    void testTicketRetrieval()
    {
        QFETCH(QString, sourceId);
        QFETCH(QVariantMap, arguments);
        QFETCH(QByteArray, postData);

        m_nam.requests.clear();
        OnlineTicketRetrievalJob job(sourceId, arguments, &m_nam);
        QSignalSpy finishedSpy(&job, &OnlineTicketRetrievalJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::PostOperation);
        QCOMPARE(m_nam.requests[0].data, postData);
    }

    void testDbNext()
    {
        m_nam.requests.clear();
        // moc chokes on using a raw string literal here...
        m_nam.replies.push({ QNetworkReply::NoError, 200, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><rporderheadlist version=\"1.0\"><rpheader tnr=\"\" ts=\"2023-10-29T11:36:32\"/><orderheadlist><orderhead kwid=\"12345678-90ab-cdef-1234-567890abcdef\" ldt=\"2023-10-27T11:04:18\" on=\"123456789123\"/></orderheadlist></rporderheadlist>" });
        QVariantMap args({ {QStringLiteral("name"), QStringLiteral("Konqui")}, {QStringLiteral("reference"), QStringLiteral("123456789123")}});

        OnlineTicketRetrievalJob job(QStringLiteral("db"), args, &m_nam);
        QSignalSpy finishedSpy(&job, &OnlineTicketRetrievalJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 2);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::PostOperation);
        QCOMPARE(m_nam.requests[0].data, R"(<rqfindorder version="1.0"><rqheader v="23080000" os="KCI" app="NAVIGATOR"/><rqorder on="123456789123"/><authname tln="Konqui"/></rqfindorder>)");
        QCOMPARE(m_nam.requests[1].op, QNetworkAccessManager::PostOperation);
        QCOMPARE(m_nam.requests[1].data, R"(<rqorderdetails version="1.0"><rqheader v="23040000" os="KCI" app="KCI-Webservice"/><rqorder on="123456789123" kwid="12345678-90ab-cdef-1234-567890abcdef"/><authname tln="Konqui"/></rqorderdetails>)");
    }

private:
    MockNetworkAccessManager m_nam;
};

QTEST_GUILESS_MAIN(OnlineTicketRetrievalTest)

#include "onlineticketretrievaltest.moc"
