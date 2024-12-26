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

        TimelineElement group(nullptr, TimelineElement::TripGroup, dt);
        group.rangeType = TimelineElement::RangeBegin;
        QTest::newRow("transfer-group-begin") << group << TimelineElement(nullptr, TimelineElement::Transfer, dt);
        group.rangeType = TimelineElement::RangeEnd;
        QTest::newRow("transfer-group-end") << TimelineElement(nullptr, TimelineElement::Transfer, dt) << group;

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

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        ctrl->commitImport(&importer);
        QCOMPARE(insertSpy.size(), 2);
        QCOMPARE(insertSpy.at(0).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(0).at(2).toInt(), 0);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo); // DST change
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);

        insertSpy.clear();
        updateSpy.clear();
        rmSpy.clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        ctrl->commitImport(&importer);
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 0);
        QCOMPARE(model.rowCount(), 3);

        insertSpy.clear();
        updateSpy.clear();
        rmSpy.clear();
        Test::clearAll(&resMgr);
        QCOMPARE(insertSpy.size(), 0);
        QCOMPARE(updateSpy.size(), 0);
        QCOMPARE(rmSpy.size(), 2);
        QCOMPARE(model.rowCount(), 1);
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

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/haus-randa-v1.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(insertSpy.size(), 4);
        QCOMPARE(insertSpy.at(0).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(0).at(2).toInt(), 0);
        QCOMPARE(insertSpy.at(1).at(1).toInt(), 1);
        QCOMPARE(insertSpy.at(1).at(2).toInt(), 1);
        QVERIFY(updateSpy.isEmpty());
        QCOMPARE(model.rowCount(), 5);
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
        ctrl->commitImport(&importer);
        QCOMPARE(insertSpy.size(), 3);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 3);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 1);
        QCOMPARE(insertSpy.at(1).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(1).at(2).toInt(), 0);
        QCOMPARE(rmSpy.at(0).at(1), 2);
        QCOMPARE(model.rowCount(), 5);

        // delete a split element
        insertSpy.clear();
        updateSpy.clear();
        rmSpy.clear();
        const auto resId = model.data(model.index(1, 0), TimelineModel::BatchIdRole).toString();
        QVERIFY(!resId.isEmpty());
        resMgr.removeReservation(resId);
        QCOMPARE(rmSpy.size(), 4);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
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

        TimelineModel model;
        model.setCurrentDateTime({{2024, 3, 29}, {10, 21}}); // after the DST transition in the US!
        QAbstractItemModelTester tester(&model);
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        Test::waitForReset(&model);

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/flight-txl-lhr-sfo.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 8); //  1x begin group, 2x country info, 2x flights, 1x end group, 1x DST info, today marker

        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        auto countryInfo = model.index(2, 0).data(TimelineModel::LocationInformationRole).value<LocationInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Left);
        QCOMPARE(countryInfo.drivingSideDiffers(), true);
        QCOMPARE(countryInfo.powerPlugCompatibility(), LocationInformation::Incompatible);

        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        countryInfo = model.index(4, 0).data(TimelineModel::LocationInformationRole).value<LocationInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Right);
        QCOMPARE(countryInfo.drivingSideDiffers(), false);
        QCOMPARE(countryInfo.powerPlugCompatibility(), LocationInformation::Incompatible);
        countryInfo = model.index(6, 0).data(TimelineModel::LocationInformationRole).value<LocationInformation>();
        QCOMPARE(countryInfo.isDst(), true);
        QCOMPARE(countryInfo.dstDiffers(), true);
        QCOMPARE(model.index(7, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);

        // remove the GB flight should also remove the GB country info
        auto resId = model.index(1, 0).data(TimelineModel::BatchIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);

        // remove the US flight should also remove the US country info
        resId = model.index(0, 0).data(TimelineModel::BatchIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
    }

    void testWeatherElements()
    {
        using namespace KItinerary;

        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        TransferManager::clear();
        TransferManager transferMgr;
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        TimelineModel model;
        QAbstractItemModelTester tester(&model);
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        model.setWeatherForecastManager(&weatherMgr);
        Test::waitForReset(&model);
        QCOMPARE(model.rowCount(), 1); // no weather data, as we don't know where we are

        // Add an element that will result in a defined location
        GeoCoordinates geo;
        geo.setLatitude(52.0f);
        geo.setLongitude(13.0f);
        Airport a;
        a.setName(u"Berlin"_s);
        a.setGeo(geo);
        Flight f;
        f.setArrivalAirport(a);
        f.setDepartureTime(QDateTime(QDate(2018, 1, 1), QTime(0, 0)));
        a.setName(u"Zurich"_s);
        a.setGeo({});
        f.setDepartureAirport(a);
        FlightReservation res;
        res.setReservationFor(f);
        resMgr.addReservation(res);

        // if there's only <1h left in a day, we wont get a weather element
        auto currentDate = QDate::currentDate();
        if (QDateTime::currentDateTimeUtc().time().hour() >= 23) {
            currentDate = currentDate.addDays(1);
        }

        QCOMPARE(model.rowCount(), 11); // 1x flight, 1x today, 9x weather
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        auto fc = model.index(2, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), currentDate);
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(model.index(10, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(10, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), currentDate.addDays(8));

        // Add a flight one day from now changing location mid-day
        geo.setLatitude(46.0f);
        geo.setLongitude(8.0f);
        a.setGeo(geo);
        f.setArrivalAirport(a);
        f.setDepartureTime(QDateTime(currentDate.addDays(1), QTime(12, 0)));
        f.setArrivalTime(QDateTime(currentDate.addDays(1), QTime(14, 0)));
        res.setReservationFor(f);
        resMgr.addReservation(res);

        QCOMPARE(model.rowCount(), 13); // 2x flight, 1x today, 10x weather
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(2, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), currentDate);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(1), QTime(0, 0)));
        QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(5, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(5, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(1), QTime(14, 0)));
        QCOMPARE(model.index(6, 0).data(TimelineModel::ElementTypeRole), TimelineElement::WeatherForecast);
        fc = model.index(6, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(2), QTime(0, 0)));

        // check we get update signals for all weather elements
        QSignalSpy spy(&model, &TimelineModel::dataChanged);
        QVERIFY(spy.isValid());
        Q_EMIT weatherMgr.forecastUpdated();
        QCOMPARE(spy.size(), 10);
        QCOMPARE(model.rowCount(), 13); // 2x flight, 1x today, 10x weather

        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(1), QTime(0, 0)));
        fc = model.index(9, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(5), QTime(0, 0)));

        // add a location change far in the future, this must not change anything
        // NOTE: this adds visible trip grouping
        geo.setLatitude(60.0f);
        geo.setLongitude(11.0f);
        a.setGeo(geo);
        f.setArrivalAirport(a);
        f.setDepartureTime(QDateTime(currentDate.addYears(1), QTime(6, 0)));
        res.setReservationFor(f);
        resMgr.addReservation(res);
        QCOMPARE(model.rowCount(), 16);

        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(1), QTime(0, 0)));
        fc = model.index(11, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(5), QTime(0, 0)));

        // result is the same when data hasn't been added incrementally
        {
            TimelineModel model;
            QAbstractItemModelTester tester(&model);
            model.setReservationManager(&resMgr);
            model.setTransferManager(&transferMgr);
            model.setTripGroupManager(&groupMgr);
            model.setWeatherForecastManager(&weatherMgr);
            Test::waitForReset(&model);
            QCOMPARE(model.rowCount(), 16);

            fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
            QVERIFY(fc.isValid());
            QCOMPARE(fc.minimumTemperature(), 13.0f);
            QCOMPARE(fc.maximumTemperature(), 52.0f);
            QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(1), QTime(0, 0)));
            fc = model.index(11, 0).data(TimelineModel::WeatherForecastRole).value<WeatherInformation>().forecast;
            QVERIFY(fc.isValid());
            QCOMPARE(fc.minimumTemperature(), 8.0f);
            QCOMPARE(fc.maximumTemperature(), 46.0f);
            QCOMPARE(fc.dateTime(), QDateTime(currentDate.addDays(5), QTime(0, 0)));
        }

        // clean up
        auto resId = model.index(15, 0).data(TimelineModel::BatchIdRole).toString();
        resMgr.removeReservation(resId);
        resId = model.index(5, 0).data(TimelineModel::BatchIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 13);

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

        TimelineModel model;
        model.setCurrentDateTime({{2024, 3, 29}, {10, 21}}); // after the DST transition in the US!
        QAbstractItemModelTester tester(&model);
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        Test::waitForReset(&model);
        QCOMPARE(model.rowCount(), 1); // 1x TodayMarker

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        // full import at runtime
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 8); // 1x group begin, 2x Flight, 2x tz change info, 1x group end, 1x DST info, 1x TodayMarker
        QCOMPARE(updateSpy.count(), 7);
        QCOMPARE(rmSpy.count(), 3);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
        QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(6, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
        QCOMPARE(model.index(7, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
        QCOMPARE(resMgr.reservationsForBatch(model.index(1, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);
        QCOMPARE(resMgr.reservationsForBatch(model.index(3, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);

        // already existing data
        {
            TimelineModel model;
            model.setCurrentDateTime({{2024, 3, 29}, {10, 21}}); // after the DST transition in the US!
            QAbstractItemModelTester tester(&model);
            model.setReservationManager(&resMgr);
            model.setTransferManager(&transferMgr);
            model.setTripGroupManager(&groupMgr);
            Test::waitForReset(&model);

            QCOMPARE(model.rowCount(), 8); // 1x group begin, 2x Flight, 2x tz change info, 1x group end, 1x DST info, 1x TodayMarker
            QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
            QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
            QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineElement::Flight);
            QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
            QCOMPARE(model.index(6, 0).data(TimelineModel::ElementTypeRole), TimelineElement::LocationInfo);
            QCOMPARE(model.index(7, 0).data(TimelineModel::ElementTypeRole), TimelineElement::TodayMarker);
            QCOMPARE(resMgr.reservationsForBatch(model.index(1, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);
            QCOMPARE(resMgr.reservationsForBatch(model.index(3, 0).data(TimelineModel::BatchIdRole).toString()).size(), 2);
        }

        // update splits element
        updateSpy.clear();
        insertSpy.clear();
        rmSpy.clear();
        auto resId = model.index(1, 0).data(TimelineModel::BatchIdRole).toString();
        QVERIFY(!resId.isEmpty());
        auto res = resMgr.reservation(resId).value<FlightReservation>();
        auto flight = res.reservationFor().value<Flight>();
        flight.setDepartureTime(flight.departureTime().addDays(1));
        res.setReservationFor(flight);
        resMgr.updateReservation(resId, res);
        QCOMPARE(model.rowCount(), 9);
        QCOMPARE(updateSpy.count(), 5);
        QCOMPARE(insertSpy.count(), 6);
        QCOMPARE(rmSpy.count(), 5);

        // update merges two elements
        updateSpy.clear();
        insertSpy.clear();
        rmSpy.clear();
        flight.setDepartureTime(flight.departureTime().addDays(-1));
        res.setReservationFor(flight);
        resMgr.updateReservation(resId, res);
        QCOMPARE(model.rowCount(), 6);
        QCOMPARE(updateSpy.count(), 5);
        QCOMPARE(rmSpy.count(), 5);
        QCOMPARE(insertSpy.count(), 2);

        // removal of merged items
        updateSpy.clear();
        rmSpy.clear();
        Test::clearAll(&resMgr);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(rmSpy.count(), 8);
        QCOMPARE(updateSpy.count(), 6);
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
        vp3.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
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
        vp.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
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
