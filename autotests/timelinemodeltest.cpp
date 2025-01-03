/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelverificationpoint.h"
#include "testhelper.h"

#include "applicationcontroller.h"
#include "favoritelocationmodel.h"
#include "importcontroller.h"
#include "livedatamanager.h"
#include "locationinformation.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "timelinemodel.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"
#include "weatherinformation.h"

#include "weatherforecast.h"
#include "weatherforecastmanager.h"

#include <KItinerary/Flight>
#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>

#include <QAbstractItemModelTester>
#include <QDirIterator>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>
#include <QtTest/qtest.h>

using namespace Qt::Literals::StringLiterals;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "en_US");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TimelineModelTest : public QObject
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

    void testElementCompare_data()
    {
        QTest::addColumn<TimelineElement>("lhs");
        QTest::addColumn<TimelineElement>("rhs");

        const auto dt = QDateTime::currentDateTimeUtc();

        TimelineElement locInfo(nullptr, TimelineElement::LocationInfo, dt);
        TimelineElement hotel(nullptr, TimelineElement::Hotel, dt);
        hotel.rangeType = TimelineElement::RangeBegin;
        QTest::newRow("locinfo-before-hotel") << locInfo << hotel;
        TimelineElement flight(nullptr, TimelineElement::Flight, dt);
        QTest::newRow("locinfo-before-flight") << locInfo << flight;

        QTest::newRow("location-info-before-transfer")
            << TimelineElement(nullptr, TimelineElement::LocationInfo, dt) << TimelineElement(nullptr, TimelineElement::Transfer, dt);
    }

    void testElementCompare()
    {
        QFETCH(TimelineElement, lhs);
        QFETCH(TimelineElement, rhs);
        QCOMPARE(lhs < rhs, true);
        QCOMPARE(rhs < lhs, false);
    }

    void testModel()
    {
        PkPassManager mgr;
        Test::clearAll(&mgr);
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setPkPassManager(&mgr);
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;

        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);

        TimelineModel model;
        model.setCurrentDateTime({{2024, 3, 31}, {17, 18}});
        QAbstractItemModelTester tester(&model);
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        Test::waitForReset(&model);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.setTripGroupName(u"Test Group"_s);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        ctrl->commitImport(&importer);

        QCOMPARE(groupMgr.tripGroups().size(), 1);
        const auto tgId = groupMgr.tripGroups()[0];
        model.setTripGroupId(tgId);
        Test::waitForReset(&model);

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        insertSpy.clear();
        updateSpy.clear();

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        importer.setTripGroupId(tgId);
        ctrl->commitImport(&importer);
        QCOMPARE(insertSpy.size(), 0);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 0);
        QCOMPARE(model.rowCount(), 1);

        Test::clearAll(&resMgr);
        QCOMPARE(model.rowCount(), 0);
    }

    void testNestedElements()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;

        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);

        TimelineModel model;
        model.setCurrentDateTime({{2024, 3, 29}, {11, 38}});
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        Test::waitForReset(&model);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.setTripGroupName(u"Test Group"_s);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/haus-randa-v1.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(groupMgr.tripGroups().size(), 1);
        const auto tgId = groupMgr.tripGroups()[0];
        model.setTripGroupId(tgId);
        Test::waitForReset(&model);

        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Hotel);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementRangeRole), TimelineElement::RangeBegin);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Hotel);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementRangeRole), TimelineElement::RangeEnd);

        // move end date of a hotel booking: dataChanged on RangeBegin, move (or del/ins) on RangeEnd
        insertSpy.clear();
        updateSpy.clear();
        rmSpy.clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/haus-randa-v2.json")));
        importer.setTripGroupId(tgId);
        ctrl->commitImport(&importer);

        QCOMPARE(insertSpy.size(), 2);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 2);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 1);
        QCOMPARE(insertSpy.at(1).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(1).at(2).toInt(), 0);
        QCOMPARE(rmSpy.at(0).at(1), 2);
        QCOMPARE(model.rowCount(), 3);

        // delete a split element
        insertSpy.clear();
        updateSpy.clear();
        rmSpy.clear();
        const auto resId = model.data(model.index(1, 0), TimelineModel::BatchIdRole).toString();
        QVERIFY(!resId.isEmpty());
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 0);
    }

    void testCountryInfos()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);

        TimelineModel model;
        model.setCurrentDateTime({{2024, 3, 29}, {10, 21}}); // after the DST transition in the US!
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/flight-txl-lhr-sfo.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(groupMgr.tripGroups().size(), 1);
        const auto tgId = groupMgr.tripGroups()[0];
        model.setTripGroupId(tgId);
        Test::waitForReset(&model);

        QCOMPARE(model.rowCount(), 4); // 2x country info, 2x flights

        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        auto countryInfo = model.index(1, 0).data(TimelineModel::LocationInformationRole).value<LocationInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Left);
        QCOMPARE(countryInfo.drivingSideDiffers(), true);
        QCOMPARE(countryInfo.powerPlugCompatibility(), LocationInformation::Incompatible);

        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        countryInfo = model.index(3, 0).data(TimelineModel::LocationInformationRole).value<LocationInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Right);
        QCOMPARE(countryInfo.drivingSideDiffers(), false);
        QCOMPARE(countryInfo.powerPlugCompatibility(), LocationInformation::Incompatible);

        // remove the GB flight should also remove the GB country info
        auto resId = model.index(0, 0).data(TimelineModel::BatchIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);

        // remove the US flight should also remove the US country info
        resId = model.index(0, 0).data(TimelineModel::BatchIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 0);
    }

    void testWeatherElements()
    {
        using namespace KItinerary;

        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        TimelineModel model;
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({2017, 9, 11}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        model.setWeatherForecastManager(&weatherMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        importer.setTripGroupName(u"Randa 2017"_s);
        ctrl->commitImport(&importer);

        QCOMPARE(groupMgr.tripGroups().size(), 1);
        model.setTripGroupId(groupMgr.tripGroups()[0]);
        Test::waitForReset(&model);

        QCOMPARE(model.rowCount(), 23);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TrainTrip);
        QCOMPARE(model.index(6, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
        QCOMPARE(model.index(7, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        auto fc = model.index(7, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate(2017, 9, 11));
        QCOMPARE(fc.minimumTemperature(), 7.78326f);
        QCOMPARE(fc.maximumTemperature(), 46.099f);

        QCOMPARE(model.index(16, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(16, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate(2017, 9, 15));
        QCOMPARE(fc.minimumTemperature(), 7.78326f);
        QCOMPARE(fc.maximumTemperature(), 46.099f);

        QCOMPARE(model.index(19, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TrainTrip);
        QCOMPARE(model.index(20, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(20, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate(2017, 9, 15));
        QCOMPARE(fc.minimumTemperature(), 8.56222f);
        QCOMPARE(fc.maximumTemperature(), 47.4503f);

        QCOMPARE(model.index(21, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(22, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(22, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate(2017, 9, 15));
        QCOMPARE(fc.minimumTemperature(), 13.2878f);
        QCOMPARE(fc.maximumTemperature(), 52.5597f);

        // check we get update signals for all weather elements
        QSignalSpy spy(&model, &TimelineModel::dataChanged);
        QVERIFY(spy.isValid());
        Q_EMIT weatherMgr.forecastUpdated();
        QCOMPARE(model.rowCount(), 23);
        QCOMPARE(spy.size(), 9);

        // test case: two consecutive location changes, the first one to an unknown location
        // result: the weather element before the first location change ends with the start of that
        // result 2: we get a second weather element the same day after the second location change
        // TODO
    }

    // multi-day event at different location from last known location, but no explicit transfers/location changes
    // should produce weather elements on event site
    void testWeatherNoLocationChange()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        TimelineModel model;
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({2023, 3, 16}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        model.setWeatherForecastManager(&weatherMgr);
        Test::waitForReset(&model);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/weather-no-location-change.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(groupMgr.tripGroups().size(), 1);
        model.setTripGroupId(groupMgr.tripGroups()[0]);
        Test::waitForReset(&model);

        ModelVerificationPoint vp0(QLatin1StringView(SOURCE_DIR "/data/weather-no-location-change.model"));
        vp0.setRoleFilter({TimelineModel::BatchIdRole});
        vp0.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp0.verify(&model));
    }

    void testMultiTraveller()
    {
        using namespace KItinerary;

        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);

        TimelineModel model;
        model.setCurrentDateTime({{2024, 3, 29}, {10, 21}}); // after the DST transition in the US!
        QAbstractItemModelTester tester(&model);
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        const auto tgId = groupMgr.createEmptyGroup(u"Test Trip"_s);
        QVERIFY(!tgId.isEmpty());
        QCOMPARE(groupMgr.tripGroups().size(), 1);
        model.setTripGroupId(tgId);
        Test::waitForReset(&model);

        // full import at runtime
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        importer.setTripGroupId(tgId);
        ctrl->commitImport(&importer);

        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(resMgr.reservationsForBatch(model.index(0, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);
        QCOMPARE(resMgr.reservationsForBatch(model.index(2, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);

        // already existing data
        {
            TimelineModel model;
            model.setCurrentDateTime({{2024, 3, 29}, {10, 21}}); // after the DST transition in the US!
            QAbstractItemModelTester tester(&model);
            model.setReservationManager(&resMgr);
            model.setTransferManager(&transferMgr);
            model.setTripGroupManager(&groupMgr);
            model.setTripGroupId(tgId);
            Test::waitForReset(&model);

            QCOMPARE(model.rowCount(), 4); // 2x Flight, 2x tz change info
            QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
            QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
            QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
            QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
            QCOMPARE(resMgr.reservationsForBatch(model.index(0, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);
            QCOMPARE(resMgr.reservationsForBatch(model.index(2, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);
        }

        groupMgr.suspend(); // avoid auto-regrouping while we mess with things here

        // update splits element
        updateSpy.clear();
        insertSpy.clear();
        rmSpy.clear();
        const auto resId = model.index(0, 0).data(TimelineModel::BatchIdRole).toString();
        QVERIFY(!resId.isEmpty());
        const auto resIds = resMgr.reservationsForBatch(resId);
        auto res = resMgr.reservation(resId).value<FlightReservation>();
        auto flight = res.reservationFor().value<Flight>();
        flight.setDepartureTime(flight.departureTime().addDays(1));
        res.setReservationFor(flight);
        resMgr.updateReservation(resId, res);
        groupMgr.addToGroup(resIds, tgId); // manually group, as auto-grouping would reject the trip we just created here

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(updateSpy.count(), 4);
        QCOMPARE(insertSpy.count(), 3);
        QCOMPARE(rmSpy.count(), 2);

        // update merges two elements
        updateSpy.clear();
        insertSpy.clear();
        rmSpy.clear();
        flight.setDepartureTime(flight.departureTime().addDays(-1));
        res.setReservationFor(flight);
        resMgr.updateReservation(resId, res);
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(resMgr.reservationsForBatch(model.index(0, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);

        QCOMPARE(updateSpy.count(), 5);
        QCOMPARE(rmSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 0);

        // removal of merged items
        updateSpy.clear();
        rmSpy.clear();
        Test::clearAll(&resMgr);
        QCOMPARE(model.rowCount(), 0);
    }

    void testDayChange()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        importer.setTripGroupName(u"Randa 2017"_s);
        ctrl->commitImport(&importer);
        QCOMPARE(groupMgr.tripGroups().size(), 1);

        TimelineModel model;
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({2017, 9, 1}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        model.setWeatherForecastManager(&weatherMgr);
        model.setTripGroupId(groupMgr.tripGroups()[0]);
        Test::waitForReset(&model);

        // before the trip, close enough for weather but no today marker
        ModelVerificationPoint vp0(QLatin1StringView(SOURCE_DIR "/data/timeline/daychange-r0.model"));
        vp0.setRoleFilter({TimelineModel::BatchIdRole});
        vp0.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp0.verify(&model));

        // changing the day should move the today marker
        model.setCurrentDateTime(QDateTime({2017, 9, 11}, {0, 15}));
        ModelVerificationPoint vp1(QLatin1StringView(SOURCE_DIR "/data/timeline/daychange-r1.model"));
        vp1.setRoleFilter({TimelineModel::BatchIdRole});
        vp1.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp1.verify(&model));

        // changing the day should move the today marker and weather one day forward
        model.setCurrentDateTime(QDateTime({2017, 9, 12}, {19, 30}));
        ModelVerificationPoint vp3(QLatin1StringView(SOURCE_DIR "/data/timeline/daychange-r3.model"));
        vp3.setRoleFilter({TimelineModel::BatchIdRole});
        vp3.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp3.verify(&model));
    }

    void testContent_data()
    {
        QTest::addColumn<QString>("baseName");

        QDirIterator it(QLatin1StringView(SOURCE_DIR "/data/timeline/"), {QLatin1StringView("*.json")});
        while (it.hasNext()) {
            it.next();
            if (it.fileInfo().fileName().endsWith(".context.json"_L1)) {
                continue;
            }
            const auto baseName = it.fileInfo().baseName();
            QTest::newRow(baseName.toUtf8().constData()) << baseName;
        }
    }

    void testContent()
    {
        if constexpr (sizeof(double) == 4) {
            QSKIP("test data assumes 64bit double resolution - skipping");
        }
        QFETCH(QString, baseName);
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        FavoriteLocationModel favLocModel;
        while (favLocModel.rowCount()) {
            favLocModel.removeLocation(0);
        }
        FavoriteLocation favLoc;
        favLoc.setLatitude(52.51860f);
        favLoc.setLongitude(13.37630f);
        favLoc.setName(QStringLiteral("Home"));
        favLocModel.setFavoriteLocations({favLoc});
        QCOMPARE(favLocModel.rowCount(), 1);

        LiveDataManager liveMgr;

        TransferManager::clear();
        TransferManager transferMgr;
        transferMgr.overrideCurrentDateTime(QDateTime({1996, 10, 14}, {12, 34}));
        transferMgr.setReservationManager(&resMgr);
        transferMgr.setFavoriteLocationModel(&favLocModel);
        transferMgr.setLiveDataManager(&liveMgr);

        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&groupMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/") + baseName + ".context.json"_L1));
        ctrl->commitImport(&importer);

        QVariant initialLocation;
        if (!groupMgr.tripGroups().empty()) {
            const auto tg = groupMgr.tripGroup(groupMgr.tripGroups()[0]);
            QCOMPARE(tg.elements().size(), 1);
            initialLocation = KItinerary::LocationUtil::arrivalLocation(resMgr.reservation(tg.elements()[0]));
        }

        QSignalSpy tgAddSpy(&groupMgr, &TripGroupManager::tripGroupAdded);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/") + baseName + ".json"_L1));
        ctrl->commitImport(&importer);
        QCOMPARE(tgAddSpy.size(), 1);

        TimelineModel model;
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({1996, 10, 14}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTripGroupManager(&groupMgr);
        model.setWeatherForecastManager(&weatherMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupId(tgAddSpy.at(0).at(0).toString());
        model.setProperty("initialLocation", initialLocation);
        Test::waitForReset(&model);

        // check state is correct for data imported at the start
        ModelVerificationPoint vp(QLatin1StringView(SOURCE_DIR "/data/timeline/") + baseName + QLatin1StringView(".model"));
        vp.setRoleFilter({TimelineModel::BatchIdRole});
        vp.setJsonPropertyFilter({"reservationId"_L1, "elements"_L1});
        QVERIFY(vp.verify(&model));

        // retry with loading during runtime
        Test::clearAll(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/") + baseName + ".context.json"_L1));
        importer.setTripGroupName(u"Context Group"_s);
        ctrl->commitImport(&importer);
        tgAddSpy.clear();

        const auto newTgId = groupMgr.createEmptyGroup(u"Test Group"_s);
        importer.setTripGroupId(newTgId);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/") + baseName + ".json"_L1));
        ctrl->commitImport(&importer);
        QCOMPARE(tgAddSpy.size(), 1);
        QCOMPARE(tgAddSpy.at(0).at(0).toString(), newTgId);

        model.setTripGroupId(newTgId);
        Test::waitForReset(&model);
        QVERIFY(vp.verify(&model));
    }

    void testTripGroupFilter()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(tgMgr.tripGroups().size(), 2);

        TimelineModel model;
        QAbstractItemModelTester tester(&model);
        model.setReservationManager(&resMgr);
        model.setTripGroupManager(&tgMgr);
        model.setTransferManager(&transferMgr);
        QCOMPARE(model.rowCount(), 0);

        auto tgId1 = tgMgr.tripGroups()[0];
        auto tgId2 = tgMgr.tripGroups()[1];
        if (tgMgr.tripGroup(tgId1).name() == "Randa (September 2017)"_L1) {
            std::swap(tgId1, tgId2);
        }
        model.setTripGroupId(tgId1);

        Test::waitForReset(&model);
        QCOMPARE(model.rowCount(), 4);

        model.setTripGroupId(tgId2);
        QCOMPARE(model.rowCount(), 0);
        Test::waitForReset(&model);
        QCOMPARE(model.rowCount(), 12);
    }
};

QTEST_GUILESS_MAIN(TimelineModelTest)

#include "timelinemodeltest.moc"
