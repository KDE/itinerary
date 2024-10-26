/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgrouplocationmodel.h"
#include "tripgroupmanager.h"

#include <KItinerary/LocationUtil>

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;
using namespace KItinerary;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "C");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TripGroupLocationModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testModel()
    {
        TripGroupLocationModel model;
        QAbstractItemModelTester modelTest(&model);

        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager::clear();
        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        model.setTripGroupManager(&tgMgr);
        QCOMPARE(model.rowCount(), 0);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(tgMgr.tripGroups().size(), 1);
        const auto tgId = tgMgr.tripGroups().at(0);
        model.setProperty("tripGroupId", tgId);

        QCOMPARE(model.rowCount(), 8);
        for (auto i = 0; i < model.rowCount(); ++i) {
            const auto idx = model.index(i, 0);
            QVERIFY(!idx.data(TripGroupLocationModel::LocationNameRole).toString().isEmpty());
            QVERIFY(idx.data(TripGroupLocationModel::LocationRole).value<KPublicTransport::Location>().hasCoordinate());
            QVERIFY(idx.data(TripGroupLocationModel::LastUsedRole).toDateTime().isValid());
            QVERIFY(idx.data(TripGroupLocationModel::UseCountRole).toInt() > 0);
        }

        model.setProperty("tripGroupId", QString());
        QCOMPARE(model.rowCount(), 0);
    }
};
QTEST_GUILESS_MAIN(TripGroupLocationModelTest)

#include "tripgrouplocationmodeltest.moc"
