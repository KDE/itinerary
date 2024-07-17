/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroupsplitmodel.h"

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace Qt::Literals;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "C");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TripGroupSplitModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testModel()
    {
        TripGroupSplitModel model;
        QAbstractItemModelTester modelTest(&model);

        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        model.setProperty("reservationManager", QVariant::fromValue(&resMgr));

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        QStringList elems;
        std::copy(resMgr.batches().begin(), resMgr.batches().end(), std::back_inserter(elems));
        model.setElements(elems);
        QCOMPARE(model.rowCount(), 11);

        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);

        QCOMPARE(model.setData(model.index(4, 0), true, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);

        QCOMPARE(model.setData(model.index(5, 0), true, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(6, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);

        QCOMPARE(model.setData(model.index(5, 0), false, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);

        QCOMPARE(model.setData(model.index(10, 0), true, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);

        QCOMPARE(model.setData(model.index(10, 0), false, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);

        QCOMPARE(model.setData(model.index(0, 0), false, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);

        QCOMPARE(model.setData(model.index(5, 0), false, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(6, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);

        QCOMPARE(model.setData(model.index(5, 0), true, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);

        QCOMPARE(model.setData(model.index(0, 0), true, TripGroupSplitModel::SelectedRole), true);
        QCOMPARE(model.index(0, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(4, 0).data(TripGroupSplitModel::SelectedRole).toBool(), true);
        QCOMPARE(model.index(5, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);
        QCOMPARE(model.index(10, 0).data(TripGroupSplitModel::SelectedRole).toBool(), false);

    }
};
QTEST_GUILESS_MAIN(TripGroupSplitModelTest)

#include "tripgroupsplitmodeltest.moc"
