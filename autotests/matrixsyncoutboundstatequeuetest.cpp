/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncoutboundstatequeue.h"

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class MatrixSyncOutboundStateQueueTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QFile::remove(MatrixSyncOutboundStateQueue::path());
    }

    void testQueue()
    {
        MatrixSyncOutboundStateQueue q;
        QSignalSpy resSpy(&q, &MatrixSyncOutboundStateQueue::batchChanged);

        QVERIFY(q.isEmpty());
        QVERIFY(!q.isSuspended());

        q.append(MatrixSyncOutboundStateQueue::BatchChange, u"1234"_s);
        QVERIFY(!q.isEmpty());

        // persistence
        {
            MatrixSyncOutboundStateQueue q2;
            QSignalSpy resSpy2(&q2, &MatrixSyncOutboundStateQueue::batchChanged);
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
            q.append(MatrixSyncOutboundStateQueue::BatchChange, u"1234"_s);
            QVERIFY(q.isSuspended());
        }
        QVERIFY(q.isEmpty());
        QVERIFY(!q.isSuspended());
        QCOMPARE(resSpy.size(), 2);
        resSpy.clear();

        // compression
        q.append(MatrixSyncOutboundStateQueue::BatchChange, u"4321"_s);
        q.append(MatrixSyncOutboundStateQueue::BatchChange, u"4321"_s);
        q.append(MatrixSyncOutboundStateQueue::BatchChange, u"4321"_s);
        q.append(MatrixSyncOutboundStateQueue::BatchChange, u"4321"_s);
        q.replayNext();
        QVERIFY(resSpy.wait());
        q.replayNext();
        QCOMPARE(resSpy.size(), 2);
        QVERIFY(q.isEmpty());
    }
};

QTEST_GUILESS_MAIN(MatrixSyncOutboundStateQueueTest)

#include "matrixsyncoutboundstatequeuetest.moc"
