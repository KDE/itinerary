/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <applicationcontroller.h>
#include <locationinformation.h>
#include <reservationmanager.h>
#include <tripgroup.h>
#include <tripgroupmanager.h>
#include <tripgroupinfoprovider.h>
#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class TripGroupInfoProviderTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        const auto batches = mgr->batches(); // copy, as this is getting modified in the process
        for (const auto &id : batches) {
            mgr->removeBatch(id);
        }
        QCOMPARE(mgr->batches().size(), 0);
    }

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testInfoProvider()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        TripGroupManager::clear();
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        WeatherForecastManager fcMgr;
        fcMgr.setTestModeEnabled(true);
        ApplicationController ctrl;
        ctrl.setReservationManager(&resMgr);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        QCOMPARE(mgr.tripGroups().size(), 1);

        TripGroupInfoProvider provider;
        provider.setReservationManager(&resMgr);
        provider.setWeatherForecastManager(&fcMgr);
        QVERIFY(!provider.weatherForecast({}).isValid());
        const auto fc = provider.weatherForecast(mgr.tripGroup(mgr.tripGroups().at(0)));
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 7.74224f);
        QCOMPARE(fc.maximumTemperature(), 47.4647f);

        const auto countries = provider.locationInformation(mgr.tripGroup(mgr.tripGroups().at(0)), QStringLiteral("DE"));
        QCOMPARE(countries.size(), 1);
        const auto country = countries.at(0).value<LocationInformation>();
        QCOMPARE(country.isoCode(), QStringLiteral("CH"));
        QCOMPARE(country.powerPlugCompatibility(), LocationInformation::PartiallyCompatible);
    }

};

QTEST_GUILESS_MAIN(TripGroupInfoProviderTest)

#include "tripgroupinfoprovidertest.moc"
