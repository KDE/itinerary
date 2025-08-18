/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncinboundstatequeue.h"
#include "matrixsyncstateevent.h"

#include <Quotient/events/stateevent.h>

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class MatrixSyncInboundStateQueueTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testQueue()
    {
        MatrixSyncInboundStateQueue q;
        QSignalSpy downloadSpy(&q, &MatrixSyncInboundStateQueue::downloadFile);
        QSignalSpy reservationSpy(&q, &MatrixSyncInboundStateQueue::reservationEvent);
        QSignalSpy documentSpy(&q, &MatrixSyncInboundStateQueue::documentEvent);
        QSignalSpy transferSpy(&q, &MatrixSyncInboundStateQueue::transferEvent);

        q.append(Quotient::StateEvent(MatrixSync::ReservationEventType, u"12345"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "none"_L1},
            {"content"_L1, "hello"_L1}
        }}), u"room42"_s);

        QCOMPARE(downloadSpy.size(), 0);
        QCOMPARE(reservationSpy.size(), 1);
        QCOMPARE(documentSpy.size(), 0);

        q.append(Quotient::StateEvent(MatrixSync::DocumentEventType, u"12346"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "file"_L1},
            {"file"_L1, QJsonObject{{
                {"url"_L1, "mxc://bla"_L1}
            }}}
        }}), u"room42"_s);

        QCOMPARE(downloadSpy.size(), 1);
        QCOMPARE(reservationSpy.size(), 1);
        QCOMPARE(documentSpy.size(), 0);

        q.append(Quotient::StateEvent(MatrixSync::TransferEventType, u"12347"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "none"_L1},
            {"content"_L1, "hello"_L1}
        }}), u"room42"_s);
        QCOMPARE(downloadSpy.size(), 1);
        QCOMPARE(reservationSpy.size(), 1);
        QCOMPARE(documentSpy.size(), 0);
        QCOMPARE(transferSpy.size(), 0);

        q.setFileName(u"/tmp/foo.txt"_s);
        QCOMPARE(downloadSpy.size(), 1);
        QCOMPARE(reservationSpy.size(), 1);
        QCOMPARE(documentSpy.size(), 1);
        QCOMPARE(transferSpy.size(), 1);
    }

    void testKnownIds()
    {
        MatrixSyncInboundStateQueue q;
        QSignalSpy downloadSpy(&q, &MatrixSyncInboundStateQueue::downloadFile);
        QSignalSpy reservationSpy(&q, &MatrixSyncInboundStateQueue::reservationEvent);
        q.addKnownEventId(u"ABCDEF"_s);

        auto s = Quotient::StateEvent(MatrixSync::ReservationEventType, u"12345"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "none"_L1},
            {"content"_L1, "hello"_L1}
        }});
        s.addId(u"ABCDEF"_s);
        q.append(s, u"room42"_s);

        QCOMPARE(downloadSpy.size(), 0);
        QCOMPARE(reservationSpy.size(), 0);
    }

    void testCompression()
    {
        MatrixSyncInboundStateQueue q;
        QSignalSpy downloadSpy(&q, &MatrixSyncInboundStateQueue::downloadFile);
        QSignalSpy liveDataSpy(&q, &MatrixSyncInboundStateQueue::liveDataEvent);
        QSignalSpy pkPassSpy(&q, &MatrixSyncInboundStateQueue::pkPassEvent);

        // download event to stall the queue
        q.append(Quotient::StateEvent(MatrixSync::PkPassEventType, u"12345"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "file"_L1},
            {"file"_L1, QJsonObject{{
                {"url"_L1, "mxc://bla"_L1}
            }}}
        }}), u"room42"_s);

        QCOMPARE(q.size(), 1);
        QCOMPARE(downloadSpy.size(), 1);
        QCOMPARE(liveDataSpy.size(), 0);
        QCOMPARE(pkPassSpy.size(), 0);

        q.append(Quotient::StateEvent(MatrixSync::LiveDataEventType, u"12347"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "none"_L1},
            {"content"_L1, "hello"_L1}
        }}), u"room42"_s);
        QCOMPARE(q.size(), 2);
        QCOMPARE(downloadSpy.size(), 1);
        QCOMPARE(liveDataSpy.size(), 0);
        QCOMPARE(pkPassSpy.size(), 0);
        q.append(Quotient::StateEvent(MatrixSync::LiveDataEventType, u"12347"_s, QJsonObject{{
            {"version"_L1, 1},
            {"contentType"_L1, "none"_L1},
            {"content"_L1, "hello"_L1}
        }}), u"room42"_s);
        QCOMPARE(q.size(), 2);
        QCOMPARE(downloadSpy.size(), 1);
        QCOMPARE(liveDataSpy.size(), 0);
        QCOMPARE(pkPassSpy.size(), 0);
    }
};

QTEST_GUILESS_MAIN(MatrixSyncInboundStateQueueTest)

#include "matrixsyncinboundstatequeuetest.moc"
