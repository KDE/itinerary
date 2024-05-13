/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "documentsmodel.h"
#include "documentmanager.h"
#include "importcontroller.h"
#include "timelinedelegatecontroller.h"

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class DocumentsModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testReservationDocuments()
    {
        DocumentManager mgr;
        Test::clearAll(&mgr);

        DocumentsModel model;
        QAbstractItemModelTester modelTest(&model);
        model.setDocumentManager(&mgr);
        QCOMPARE(model.rowCount(), 0);

        auto ctrl = Test::makeAppController();
        ctrl->setDocumentManager(&mgr);
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/iata-bcbp-demo.pdf")));
        ctrl->commitImport(&importer);
        QCOMPARE(resMgr.batches().size(), 1);
        QCOMPARE(mgr.documents().size(), 1);

        TimelineDelegateController c;
        c.setReservationManager(&resMgr);
        c.setDocumentManager(&mgr);
        c.setBatchId(resMgr.batches().at(0));
        QCOMPARE(c.documentIds().size(), 1);

        model.setDocumentIds(c.documentIds());
        QCOMPARE(model.rowCount(), 1);
        const auto idx = model.index(0, 0);
        QCOMPARE(idx.data(Qt::DisplayRole), QLatin1StringView("iata-bcbp-demo.pdf"));
        QCOMPARE(idx.data(Qt::DecorationRole), QLatin1StringView("application-pdf"));

        model.setDocumentIds({});
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(DocumentsModelTest)

#include "documentsmodeltest.moc"
