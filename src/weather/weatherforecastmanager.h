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

#ifndef WEATHERFORECASTMANAGER_H
#define WEATHERFORECASTMANAGER_H

#include <QObject>

#include <deque>
#include <vector>

class WeatherForecast;
struct WeatherTile;

class QNetworkAccessManager;
class QNetworkReply;
class QXmlStreamReader;

/** Access to weather forecast data based on geo coorinates. */
class WeatherForecastManager : public QObject
{
    Q_OBJECT
public:
    explicit WeatherForecastManager(QObject *parent = nullptr);
    ~WeatherForecastManager();

    /** Monitor the specified location for weather forecasts. */
    void monitorLocation(float latitude, float longitude);
    // TODO unmonitor location(s)?

    /** Get the forecast for the given time and location. */
    Q_INVOKABLE QVariant forecast(float latitude, float longitude, const QDateTime &dt) const;

signals:
    /** Updated when new forecast data has been retrieved. */
    void forecastUpdated();

private:
    void fetchTile(WeatherTile tile);
    void fetchNext();
    void tileDownloaded();
    QString cachePath(WeatherTile tile) const;
    void writeToCacheFile(QNetworkReply *reply) const;

    void mergeForecasts(std::vector<WeatherForecast> &forecasts) const;
    std::vector<WeatherForecast> parseForecast(QXmlStreamReader &reader) const;
    WeatherForecast parseForecastElement(QXmlStreamReader &reader) const;

    std::vector<WeatherTile> m_monitoredTiles;
    std::deque<WeatherTile> m_pendingTiles;

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_pendingReply = nullptr;
};

#endif // WEATHERFORECASTMANAGER_H
