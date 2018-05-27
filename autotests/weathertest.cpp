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

#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

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

    void testForecastRetrieval()
    {
        WeatherForecastManager mgr;
        const auto now = QDateTime::currentDateTimeUtc().addSecs(1800);
        auto fc = mgr.forecast(46.1, 7.78, now).value<WeatherForecast>();
        QVERIFY(!fc.isValid());

        QSignalSpy updateSpy(&mgr, &WeatherForecastManager::forecastUpdated);
        QVERIFY(updateSpy.isValid());

        mgr.monitorLocation(46.1, 7.78);
        QVERIFY(updateSpy.wait());

        fc = mgr.forecast(46.1, 7.78, now).value<WeatherForecast>();
        qDebug() << fc.dateTime() << fc.temperature() << fc.symbolType() << fc.symbolIconName();
        QVERIFY(fc.isValid());
        QVERIFY(fc.dateTime().isValid());
        QVERIFY(fc.dateTime() <= now);
        QVERIFY(fc.symbolType() != WeatherForecast::Unknown);
        QVERIFY(fc.temperature() > -50);
        QVERIFY(fc.temperature() < 50);
    }
};

QTEST_GUILESS_MAIN(WeatherTest)

#include "weathertest.moc"
