/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncstateevent.h"

#include <Quotient/events/stateevent.h>

#include <QFile>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class MatrixSyncStateEventTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testStateEventOutbound()
    {
        auto ev = MatrixSyncStateEvent(MatrixSync::ReservationEventType, u"12345"_s);
        QVERIFY(!ev.needsDownload());
        QVERIFY(!ev.needsUpload());
        QVERIFY(!ev.hasContent());
        {
            auto state = ev.toQuotient();
            QCOMPARE(state.matrixType(), MatrixSync::ReservationEventType);
            QCOMPARE(state.stateKey(), "12345"_L1);
            QCOMPARE(state.contentJson().value("version"_L1).toInt(), 1);
            QCOMPARE(state.contentJson().value("content"_L1).toString(), QString());
        }

        ev.setContent("ABC");
        QVERIFY(!ev.needsDownload());
        QVERIFY(!ev.needsUpload());
        QVERIFY(ev.hasContent());
        {
            auto state = ev.toQuotient();
            QCOMPARE(state.contentJson().value("contentType"_L1).toString(), "none"_L1);
            QCOMPARE(state.contentJson().value("content"_L1).toString(), "ABC"_L1);
        }
        QCOMPARE(ev.content(), "ABC");

        ev.setContent(QByteArray(32, '\0'));
        {
            auto state = ev.toQuotient();
            QCOMPARE(state.contentJson().value("contentType"_L1).toString(), "base64"_L1);
            QCOMPARE(state.contentJson().value("content"_L1).toString(), "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA="_L1);
        }
        QCOMPARE(ev.content(), QByteArray(32, '\0'));

        ev = MatrixSyncStateEvent(MatrixSync::ReservationEventType, u"12345"_s);
        ev.setContent(QByteArray(65523, '\0'));
        ev.setExtraData("metaData"_L1, "BBB"_L1);
        QVERIFY(!ev.needsDownload());
        QVERIFY(ev.needsUpload());
        QVERIFY(!ev.fileName().isEmpty());

        Quotient::EncryptedFileMetadata md;
        md.url = QUrl(u"mxc://foo"_s);
        ev.setFileInfo(md);
        QVERIFY(!ev.needsUpload());
        QVERIFY(ev.hasContent());
        {
            auto state = ev.toQuotient();
            QCOMPARE(state.contentJson().value("contentType"_L1).toString(), "file"_L1);
            QCOMPARE(state.contentJson().contains("content"_L1), false);
            QCOMPARE(state.contentJson().value("file"_L1).toObject().value("url"_L1).toString(), "mxc://foo"_L1);
            QCOMPARE(state.contentJson().value("extra"_L1).toObject(), QJsonObject({{ "metaData"_L1, "BBB"_L1}}));
        }

        // small file inline storage
        ev = MatrixSyncStateEvent(MatrixSync::ReservationEventType, u"12345"_s);
        ev.setFileName(SOURCE_DIR "/data/boardingpass-v1.pkpass"_L1);
        QVERIFY(!ev.needsDownload());
        QVERIFY(!ev.needsUpload());
        QVERIFY(!ev.fileName().isEmpty());
        QVERIFY(ev.hasContent());
        {
            auto state = ev.toQuotient();
            QCOMPARE(state.contentJson().value("contentType"_L1).toString(), "base64"_L1);
            QCOMPARE(state.contentJson().contains("content"_L1), true);
        }

        // large file needing upload
        ev = MatrixSyncStateEvent(MatrixSync::ReservationEventType, u"12345"_s);
        ev.setFileName(SOURCE_DIR "/data/524-135-forecast.xml"_L1);
        QVERIFY(!ev.needsDownload());
        QVERIFY(ev.needsUpload());
        QVERIFY(ev.fileName().endsWith("524-135-forecast.xml"_L1));
        QVERIFY(ev.hasContent());
    }

    void testStateEventInbound()
    {
        auto ev = MatrixSyncStateEvent::fromQuotient(Quotient::StateEvent(MatrixSync::LiveDataEventType, u"12345"_s, QJsonObject{{
            { "contentType"_L1, "none"_L1},
            { "content"_L1, "test"_L1 },
        }}), u"room42"_s);
        QVERIFY(!ev);

        ev = MatrixSyncStateEvent::fromQuotient(Quotient::StateEvent(MatrixSync::LiveDataEventType, u"12345"_s, QJsonObject{{
            { "version"_L1, 1},
            { "contentType"_L1, "none"_L1},
            { "content"_L1, "test"_L1 },
        }}), u"room42"_s);
        QVERIFY(ev);
        QVERIFY(!ev->needsDownload());
        QVERIFY(!ev->needsUpload());
        QCOMPARE(ev->type(), MatrixSync::LiveDataEventType);
        QCOMPARE(ev->stateKey(), "12345"_L1);
        QCOMPARE(ev->content(), "test");
        QCOMPARE(ev->roomId(), "room42"_L1);
        QVERIFY(!ev->fileName().isEmpty());
        QVERIFY(ev->hasContent());
        {
            QFile f(ev->fileName());
            QVERIFY(f.open(QFile::ReadOnly));
            QCOMPARE(f.readAll(), "test");
        }

        ev = MatrixSyncStateEvent::fromQuotient(Quotient::StateEvent(MatrixSync::LiveDataEventType, u"12345"_s, QJsonObject{{
            { "version"_L1, 1},
            { "contentType"_L1, "file"_L1},
            { "file"_L1, QJsonObject{{
                { "url"_L1, "mxc://foo"_L1 },
            }}},
            { "extra"_L1, QJsonObject{{
                { "metaData"_L1, "ABC"_L1 }
            }}}
        }}), u"room42"_s);
        QVERIFY(ev);
        QVERIFY(ev->needsDownload());
        QVERIFY(!ev->needsUpload());
        QVERIFY(ev->hasContent());

        QTemporaryFile f;
        QVERIFY(f.open());
        f.write("external content");
        f.close();
        ev->setFileName(f.fileName());
        QVERIFY(!ev->needsDownload());
        QVERIFY(!ev->needsUpload());
        QCOMPARE(ev->content(), "external content");
        QCOMPARE(ev->extraData("metaData"_L1), "ABC"_L1);
        QVERIFY(ev->hasContent());
    }
};

QTEST_GUILESS_MAIN(MatrixSyncStateEventTest)

#include "matrixsyncstateeventtest.moc"
