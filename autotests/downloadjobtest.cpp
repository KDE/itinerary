/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"
#include "mocknetworkaccessmanager.h"

#include <downloadjob.h>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

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
        DownloadJob job(QUrl(QStringLiteral("https://akademy.kde.org/2023/")), &m_nam);
        QSignalSpy finishedSpy(&job, &DownloadJob::finished);
        QVERIFY(finishedSpy.wait());

        QCOMPARE(m_nam.requests.size(), 1);
        QCOMPARE(m_nam.requests[0].op, QNetworkAccessManager::GetOperation);
        QCOMPARE(m_nam.requests[0].request.url(), QUrl(QStringLiteral("https://akademy.kde.org/2023/")));
    }

private:
    MockNetworkAccessManager m_nam;
};

QTEST_GUILESS_MAIN(DownloadJobTest)

#include "downloadjobtest.moc"
