/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "itinerary_version.h"
#include "config-weather.h"

#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QXmlStreamReader>

class WeatherTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
        QCoreApplication::setApplicationName(QStringLiteral("kde-itinerary-weatherunittest"));
        QCoreApplication::setApplicationVersion(QStringLiteral(ITINERARY_VERSION_STRING));

        QStandardPaths::setTestModeEnabled(true);
        QDir d(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/weather"));
        d.removeRecursively();
        QVERIFY(!d.exists());
    }

    void testParseForecastData()
    {
        // NOTE: the data file is transformed 100 years into the future to avoid the parser cutting of outdated data
        QFile f(QLatin1String(SOURCE_DIR "/data/524-135-forecast.xml"));
        QVERIFY(f.open(QFile::ReadOnly));
        WeatherForecastManager mgr;
        QXmlStreamReader reader(&f);
        auto forecasts = mgr.parseForecast(reader, {52.4, 13.5});
        QCOMPARE(forecasts.size(), 1034);

        mgr.mergeForecasts(forecasts);
        QCOMPARE(forecasts.size(), 233);

        for (unsigned int i = 0; i < (forecasts.size() - 1); ++i) {
            QVERIFY(forecasts[i].dateTime() < forecasts[i+1].dateTime());
        }

        QDateTime dt(QDate(2118, 7, 26), QTime(6,0), Qt::UTC);
        auto it = std::lower_bound(forecasts.begin(), forecasts.end(), dt, [](const WeatherForecast &lhs, const QDateTime &rhs) {
            return lhs.dateTime() < rhs;
        });
        QCOMPARE((*it).minimumTemperature(), 21.6f);
        QCOMPARE((*it).maximumTemperature(), 21.6f);
        QCOMPARE((*it).precipitation(), 0.0f);
        QCOMPARE((*it).symbolType(), WeatherForecast::Clear);
    }

    void testWeatherSymbol()
    {
#ifdef HAVE_KHOLIDAYS
        WeatherForecast fc;
        fc.setDateTime({QDate(2018, 8, 1), QTime(2, 0), Qt::UTC}); // CEST is UTC +2
        fc.setTile({52.4, 13.5});
        fc.setRange(1);
        fc.setSymbolType(WeatherForecast::Clear);
        QCOMPARE(fc.symbolIconName(), QStringLiteral("weather-clear-night"));
        fc.setRange(2);
        QCOMPARE(fc.symbolIconName(), QStringLiteral("weather-clear"));
        fc.setRange(19);
        fc.setSymbolType(WeatherForecast::Clear | WeatherForecast::LightClouds);
        QCOMPARE(fc.symbolIconName(), QStringLiteral("weather-few-clouds"));
        fc.setDateTime({QDate(2018, 8, 1), QTime(21, 0), Qt::UTC});
        fc.setRange(1);
        QCOMPARE(fc.symbolIconName(), QStringLiteral("weather-few-clouds-night"));

        fc.setDateTime({QDate(2018, 8, 1), QTime(22, 0), Qt::UTC});
        fc.setRange(24);
        QCOMPARE(fc.symbolIconName(), QStringLiteral("weather-few-clouds"));
#endif
    }

    void testForecastRetrieval()
    {
        WeatherForecastManager mgr;
        mgr.setAllowNetworkAccess(true);
        const auto now = QDateTime::currentDateTimeUtc().addSecs(1800);
        auto fc = mgr.forecast(46.1, 7.78, now);
        QVERIFY(!fc.isValid());

        QSignalSpy updateSpy(&mgr, &WeatherForecastManager::forecastUpdated);
        QVERIFY(updateSpy.isValid());

        mgr.monitorLocation(46.1, 7.78);
        QVERIFY(updateSpy.wait());

        fc = mgr.forecast(46.1, 7.78, now);
        qDebug() << fc.dateTime() << fc.minimumTemperature() << fc.maximumTemperature() << fc.symbolType() << fc.symbolIconName();
        QVERIFY(fc.isValid());
        QVERIFY(fc.dateTime().isValid());
        QVERIFY(fc.dateTime() <= now);
        QVERIFY(fc.symbolType() != WeatherForecast::None);
        QVERIFY(fc.minimumTemperature() > -50);
        QVERIFY(fc.minimumTemperature() < 50);
        QVERIFY(fc.maximumTemperature() > -50);
        QVERIFY(fc.maximumTemperature() < 50);
        QVERIFY(fc.maximumTemperature() >= fc.minimumTemperature());
        QVERIFY(fc.precipitation() >= 0.0f);
    }
};

QTEST_GUILESS_MAIN(WeatherTest)

#include "weathertest.moc"
