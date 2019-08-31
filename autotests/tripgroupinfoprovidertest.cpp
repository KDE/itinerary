/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <countryinformation.h>
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

    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
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

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        QCOMPARE(mgr.tripGroups().size(), 1);

        TripGroupInfoProvider provider;
        provider.setReservationManager(&resMgr);
        provider.setWeatherForecastManager(&fcMgr);
        QVERIFY(!provider.weatherForecast({}).isValid());
        const auto fc = provider.weatherForecast(mgr.tripGroup(mgr.tripGroups().at(0)));
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 7.74224f);
        QCOMPARE(fc.maximumTemperature(), 47.4647f);

        const auto countries = provider.countryInformation(mgr.tripGroup(mgr.tripGroups().at(0)), QStringLiteral("DE"));
        QCOMPARE(countries.size(), 1);
        const auto country = countries.at(0).value<CountryInformation>();
        QCOMPARE(country.isoCode(), QStringLiteral("CH"));
        QCOMPARE(country.powerPlugCompatibility(), CountryInformation::PartiallyCompatible);
    }

};

QTEST_GUILESS_MAIN(TripGroupInfoProviderTest)

#include "tripgroupinfoprovidertest.moc"
