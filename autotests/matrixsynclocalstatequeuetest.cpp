/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsynccontent.h"
#include "matrixsynclocalstatequeue.h"

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class MatrixSyncLocalStateQueueTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTest()
    {
        QStandardPaths::setTestModeEnabled(true);
        QFile::remove(MatrixSyncLocalStateQueue::path());
    }

    void testQueue()
    {
        MatrixSyncLocalStateQueue q;
        QSignalSpy resSpy(&q, &MatrixSyncLocalStateQueue::batchChanged);

        QVERIFY(q.isEmpty());
        QVERIFY(!q.isSuspended());

        q.append(MatrixSyncLocalStateQueue::BatchChange, u"1234"_s);
        QVERIFY(!q.isEmpty());

        // persistence
        {
            MatrixSyncLocalStateQueue q2;
            QSignalSpy resSpy2(&q2, &MatrixSyncLocalStateQueue::batchChanged);
            QVERIFY(!q2.isEmpty());
            QVERIFY(!q2.isSuspended());
            QCOMPARE(resSpy2.size(), 0);
            q2.retry();
            QCOMPARE(resSpy2.size(), 1);
            QCOMPARE(resSpy2.at(0).at(0), "1234"_L1);
            q2.replayNext();
            QCOMPARE(q2.isEmpty(), true);
        }

        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(resSpy.at(0).at(0), "1234"_L1);
        QVERIFY(!q.isEmpty());
        q.retry();
        QCOMPARE(resSpy.size(), 2);
        q.replayNext();
        QVERIFY(q.isEmpty());
        QCOMPARE(resSpy.size(), 2);

        // suspension
        {
            MatrixSyncLocalChangeLock locker(&q);
            q.append(MatrixSyncLocalStateQueue::BatchChange, u"1234"_s);
            QVERIFY(q.isSuspended());
        }
        QVERIFY(q.isEmpty());
        QVERIFY(!q.isSuspended());
        QCOMPARE(resSpy.size(), 2);
        resSpy.clear();

        // compression
        q.append(MatrixSyncLocalStateQueue::BatchChange, u"4321"_s);
        q.append(MatrixSyncLocalStateQueue::BatchChange, u"4321"_s);
        q.append(MatrixSyncLocalStateQueue::BatchChange, u"4321"_s);
        q.append(MatrixSyncLocalStateQueue::BatchChange, u"4321"_s);
        q.replayNext();
        QVERIFY(resSpy.wait());
        q.replayNext();
        QCOMPARE(resSpy.size(), 2);
        QVERIFY(q.isEmpty());
    }
};

QTEST_GUILESS_MAIN(MatrixSyncLocalStateQueueTest)

#include "matrixsynclocalstatequeuetest.moc"
