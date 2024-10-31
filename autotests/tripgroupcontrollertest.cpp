/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "locationinformation.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupcontroller.h"
#include "tripgroupmanager.h"
#include "tripgroupmodel.h"
#include "weatherforecast.h"
#include "weatherforecastmanager.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class TripGroupControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testController()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        WeatherForecastManager fcMgr;
        fcMgr.setTestModeEnabled(true);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->setTripGroupManager(&mgr);

        TripGroupModel tgModel;
        tgModel.setTripGroupManager(&mgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.tripGroups().size(), 1);

        TripGroupController controller;
        controller.setProperty("tripGroupId", mgr.tripGroups().at(0));
        // must not crash with partial setup
        QCOMPARE(controller.canMerge(), false);
        QCOMPARE(controller.canSplit(), false);
        QVERIFY(!controller.weatherForecast().isValid());

        controller.setTripGroupModel(&tgModel);
        controller.setWeatherForecastManager(&fcMgr);
        const auto fc = controller.weatherForecast();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 7.74224f);
        QCOMPARE(fc.maximumTemperature(), 52.5597f);

        controller.setProperty("homeCountryIsoCode", u"DE"_s);
        const auto countries = controller.locationInformation();
        QCOMPARE(countries.size(), 1);
        const auto country = countries.at(0).value<LocationInformation>();
        QCOMPARE(country.isoCode(), "CH"_L1);
        QCOMPARE(country.powerPlugCompatibility(), LocationInformation::PartiallyCompatible);

        controller.setProperty("homeCurrency", u"EUR"_s);
        QCOMPARE(controller.currencies(), QStringList({u"CHF"_s}));
        controller.setProperty("homeCurrency", u"NZD"_s);
        QCOMPARE(controller.currencies(), QStringList({u"CHF"_s, u"EUR"_s}));

        QCOMPARE(controller.canSplit(), true);
        QCOMPARE(controller.canMerge(), false);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(controller.canMerge(), true);
    }
};

QTEST_GUILESS_MAIN(TripGroupControllerTest)

#include "tripgroupcontrollertest.moc"
