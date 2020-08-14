/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEATHERFORECASTMANAGER_H
#define WEATHERFORECASTMANAGER_H

#include "weathertile.h"

#include <QObject>
#include <QTimer>

#include <deque>
#include <unordered_map>
#include <vector>

class WeatherForecast;

class QNetworkAccessManager;
class QNetworkReply;
class QXmlStreamReader;

/** Access to weather forecast data based on geo coordinates. */
class WeatherForecastManager : public QObject
{
    Q_OBJECT
public:
    explicit WeatherForecastManager(QObject *parent = nullptr);
    ~WeatherForecastManager();

    /** Kill switch for network operations. */
    bool allowNetworkAccess() const;
    void setAllowNetworkAccess(bool enabled);

    /** Monitor the specified location for weather forecasts. */
    void monitorLocation(float latitude, float longitude);

    /** Get the forecast for the given time and location. */
    WeatherForecast forecast(float latitude, float longitude, const QDateTime &dt) const;
    /** Get the forecast for the give time range and location. */
    WeatherForecast forecast(float latitude, float longitude, const QDateTime &begin, const QDateTime &end) const;

    /** Time until when we have forecast data. */
    QDateTime maximumForecastTime(const QDate &today) const;

    /** Enable unit test mode.
     *  In this mode static forecast data is provided for all locations.
     */
    void setTestModeEnabled(bool testMode);

Q_SIGNALS:
    /** Updated when new forecast data has been retrieved. */
    void forecastUpdated();

private:
    friend class WeatherTest;

    void fetchTile(WeatherTile tile);
    void fetchNext();
    void tileDownloaded();
    QString cachePath(WeatherTile tile) const;
    void writeToCacheFile(QNetworkReply *reply) const;

    bool loadForecastData(WeatherTile tile) const;
    void mergeForecasts(std::vector<WeatherForecast> &forecasts) const;
    std::vector<WeatherForecast> parseForecast(QXmlStreamReader &reader, WeatherTile tile) const;
    WeatherForecast parseForecastElement(QXmlStreamReader &reader) const;

    void scheduleUpdate();
    void updateAll();
    void purgeCache();

    std::vector<WeatherTile> m_monitoredTiles;
    std::deque<WeatherTile> m_pendingTiles;
    mutable std::unordered_map<WeatherTile, std::vector<WeatherForecast>> m_forecastData;

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_pendingReply = nullptr;
    QTimer m_updateTimer;
    bool m_allowNetwork = false;
    bool m_testMode = false;
};

#endif // WEATHERFORECASTMANAGER_H
