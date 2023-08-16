/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "weatherforecastmanager.h"
#include "weatherforecast.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QVariant>
#include <QXmlStreamReader>

#include <zlib.h>

#include <cmath>

static void alignToHour(QDateTime &dt)
{
    dt.setTime(QTime(dt.time().hour(), 0, 0, 0));
}

static void roundToHour(QDateTime &dt)
{
    if (dt.time().minute() >= 30) {
        alignToHour(dt);
        dt = dt.addSecs(3600);
    } else {
        alignToHour(dt);
    }
}

/*
 * ATTENTION!
 * Before touching anything in here, especially regarding the network operations
 * make sure to read and understand https://api.met.no/conditions_service.html!
 */

WeatherForecastManager::WeatherForecastManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_updateTimer, &QTimer::timeout, this, &WeatherForecastManager::updateAll);
    m_updateTimer.setSingleShot(true);
}

WeatherForecastManager::~WeatherForecastManager() = default;

bool WeatherForecastManager::allowNetworkAccess() const
{
    return m_allowNetwork || m_testMode;
}

void WeatherForecastManager::setAllowNetworkAccess(bool enabled)
{
    m_allowNetwork = enabled;
    Q_EMIT forecastUpdated();
    if (enabled) {
        scheduleUpdate();
    } else {
        m_updateTimer.stop();
    }
    fetchNext();
}

void WeatherForecastManager::monitorLocation(float latitude, float longitude)
{
    WeatherTile t{latitude, longitude};
    qDebug() << latitude << longitude << t.lat << t.lon;

    auto it = std::lower_bound(m_monitoredTiles.begin(), m_monitoredTiles.end(), t);
    if (it != m_monitoredTiles.end() && (*it) == t) {
        return;
    }

    m_monitoredTiles.insert(it, t);
    fetchTile(t);
}

WeatherForecast WeatherForecastManager::forecast(float latitude, float longitude, const QDateTime &dt) const
{
    return forecast(latitude, longitude, dt, dt.addSecs(3600));
}

WeatherForecast WeatherForecastManager::forecast(float latitude, float longitude, const QDateTime &begin, const QDateTime &end) const
{
    if (Q_UNLIKELY(m_testMode)) {
        WeatherForecast fc;
        auto beginDt = begin;
        roundToHour(beginDt);
        fc.setDateTime(beginDt);
        auto endDt = end;
        roundToHour(endDt);
        if (beginDt == endDt) {
            endDt = endDt.addSecs(3600);
        }
        const auto range = beginDt.secsTo(endDt) / 3600;
        fc.setRange(range);
        fc.setTile({latitude, longitude});
        fc.setMinimumTemperature(std::min(latitude, longitude));
        fc.setMaximumTemperature(std::max(latitude, longitude));
        fc.setPrecipitation(23.0f);
        fc.setSymbolType(WeatherForecast::LightClouds);
        return fc;
    }

    const auto now = QDateTime::currentDateTimeUtc();
    if (end < now) {
        return {};
    }

    auto beginDt = std::max(begin, now);
    roundToHour(beginDt);
    auto endDt = std::max(end, now);
    roundToHour(endDt);
    if (!beginDt.isValid() || !endDt.isValid() || beginDt > endDt) {
        return {};
    }
    if (beginDt == endDt) {
        endDt = endDt.addSecs(3600);
    }

    WeatherTile tile{latitude, longitude};
    if (!loadForecastData(tile)) {
        return {};
    }

    const auto &forecasts = m_forecastData[tile];
    const auto beginIt = std::lower_bound(forecasts.begin(), forecasts.end(), beginDt, [](const WeatherForecast &lhs, const QDateTime &rhs) {
        return lhs.dateTime() < rhs;
    });
    const auto endIt = std::lower_bound(forecasts.begin(), forecasts.end(), endDt, [](const WeatherForecast &lhs, const QDateTime &rhs) {
        return lhs.dateTime() < rhs;
    });
    if (beginIt == forecasts.end() || beginIt == endIt) {
        return {};
    }
    const auto range = beginDt.secsTo(std::min((*std::prev(endIt)).dateTime().addSecs(3600), endDt)) / 3600;

    WeatherForecast fc(*beginIt);
    fc.setRange(range);
    for (auto it = beginIt; it != endIt; ++it) {
        fc.merge(*it);
    }
    return fc;
}

void WeatherForecastManager::fetchTile(WeatherTile tile)
{
    QFileInfo fi(cachePath(tile) + QLatin1String("forecast.xml"));
    if (fi.exists() && fi.lastModified().toUTC().addSecs(3600 * 2) >= QDateTime::currentDateTimeUtc()) { // cache is already new enough
        return;
    }

    m_pendingTiles.push_back(tile);
    fetchNext();
}

void WeatherForecastManager::fetchNext()
{
    if (!m_allowNetwork || m_pendingReply || m_pendingTiles.empty()) {
        return;
    }

    const auto tile = m_pendingTiles.front();
    m_pendingTiles.pop_front();

    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("api.met.no"));
    url.setPath(QStringLiteral("/weatherapi/locationforecast/2.0/classic"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("lat"), QString::number(tile.latitude()));
    query.addQueryItem(QStringLiteral("lon"), QString::number(tile.longitude()));
    url.setQuery(query);

    qDebug() << url;
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setAttribute(QNetworkRequest::User, QVariant::fromValue(tile));

    // see §Identification on https://api.met.no/conditions_service.html
    req.setHeader(QNetworkRequest::UserAgentHeader, QString(QCoreApplication::applicationName() +
        QLatin1Char(' ') + QCoreApplication::applicationVersion() + QLatin1String(" (kde-pim@kde.org)")));
    // TODO see §Cache on https://api.met.no/conditions_service.html
    // see §Compression on https://api.met.no/conditions_service.html
    req.setRawHeader("Accept-Encoding", "gzip");

    m_pendingReply = m_nam->get(req);
    connect(m_pendingReply, &QNetworkReply::finished, this, &WeatherForecastManager::tileDownloaded);
}

void WeatherForecastManager::tileDownloaded()
{
    // TODO handle 304 Not Modified
    // TODO handle 429 Too Many Requests
    if (m_pendingReply->error() != QNetworkReply::NoError) {
        qWarning() << m_pendingReply->errorString();
    } else {
        writeToCacheFile(m_pendingReply);
    }

    m_pendingReply->deleteLater();
    m_pendingReply = nullptr;
    if (m_pendingTiles.empty()) {
        Q_EMIT forecastUpdated();
    }
    fetchNext();
}

QString WeatherForecastManager::cachePath(WeatherTile tile) const
{
    const auto path = QString(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QLatin1String("/weather/")
        + QString::number(tile.lat) + QLatin1Char('/')
        + QString::number(tile.lon) + QLatin1Char('/'));
    QDir().mkpath(path);
    return path;
}

void WeatherForecastManager::writeToCacheFile(QNetworkReply* reply) const
{
    const auto tile = reply->request().attribute(QNetworkRequest::User).value<WeatherTile>();
    qDebug() << tile.lat << tile.lon;
    qDebug() << reply->rawHeaderPairs();
    QFile f(cachePath(tile) + QLatin1String("forecast.xml"));
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "Failed to open weather cache location:" << f.errorString();
        return;
    }

    const auto contentEncoding = reply->rawHeader("Content-Encoding");
    if (contentEncoding == "gzip") {
        const auto data = reply->readAll();
        if (data.size() < 4 || data.at(0) != 0x1f || data.at(1) != char(0x8b)) {
            qWarning() << "Invalid gzip format";
            return;
        }

        z_stream stream;
        unsigned char buffer[1024];

        stream.zalloc = nullptr;
        stream.zfree = nullptr;
        stream.opaque = nullptr;
        stream.avail_in = data.size();
        stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data()));

        auto ret = inflateInit2(&stream, 15 + 32); // see docs, the magic numbers enable gzip decoding
        if (ret != Z_OK) {
            qWarning() << "Failed to initialize zlib stream.";
            return;
        }

        do {
            stream.avail_out = sizeof(buffer);
            stream.next_out = buffer;

            ret = inflate(&stream, Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END) {
                qWarning() << "Zlib decoding failed!" << ret;
                break;
            }

            f.write(reinterpret_cast<char*>(buffer), sizeof(buffer) - stream.avail_out);
        } while (stream.avail_out == 0);
        inflateEnd(&stream);
    } else {
        f.write(reply->readAll());
    }

    m_forecastData.erase(tile);
}

bool WeatherForecastManager::loadForecastData(WeatherTile tile) const
{
    const auto it = m_forecastData.find(tile);
    if (it != m_forecastData.end()) {
        return true;
    }

    QFile f(cachePath(tile) + QLatin1String("forecast.xml"));
    if (!f.exists() || !f.open(QFile::ReadOnly)) {
        return false;
    }

    QXmlStreamReader reader(&f);
    auto forecasts = parseForecast(reader, tile);
    mergeForecasts(forecasts);
    if (forecasts.empty()) {
        return false;
    }

    m_forecastData.insert(it, {tile, std::move(forecasts)});
    return true;
}

void WeatherForecastManager::mergeForecasts(std::vector<WeatherForecast>& forecasts) const
{
    std::stable_sort(forecasts.begin(), forecasts.end(), [](const WeatherForecast &lhs, const WeatherForecast &rhs) {
        if (lhs.dateTime() == rhs.dateTime())
            return lhs.range() < rhs.range();
        return lhs.dateTime() < rhs.dateTime();
    });

    // merge duplicated time slices
    auto storeIt = forecasts.begin();
    for (auto it = forecasts.begin(); it != forecasts.end();) {
        (*storeIt) = (*it);
        auto mergeIt = it;
        for (; mergeIt != forecasts.end(); ++mergeIt) {
            if ((*it).dateTime() == (*mergeIt).dateTime()) {
                (*storeIt).merge(*mergeIt);
            } else {
                (*mergeIt).setRange(1);
                break;
            }
        }
        ++storeIt;
        it = mergeIt;
    }
    forecasts.erase(storeIt, forecasts.end());
}

std::vector<WeatherForecast> WeatherForecastManager::parseForecast(QXmlStreamReader &reader, WeatherTile tile) const
{
    std::vector<WeatherForecast> result;

    auto beginDt = QDateTime::currentDateTimeUtc();
    alignToHour(beginDt);

    while (!reader.atEnd()) {
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("weatherdata") || reader.name() == QLatin1String("product")) {
                reader.readNext(); // enter these elements
                continue;
            }
            if (reader.name() == QLatin1String("time") && reader.attributes().value(QLatin1String("datatype")) == QLatin1String("forecast")) {
                // normalize time ranges to 1 hour
                auto from = QDateTime::fromString(reader.attributes().value(QLatin1String("from")).toString(), Qt::ISODate);
                from = std::max(from, beginDt);
                alignToHour(from);
                auto to = QDateTime::fromString(reader.attributes().value(QLatin1String("to")).toString(), Qt::ISODate);
                alignToHour(to);
                const auto range = from.secsTo(to) / 3600;
                if (to == from) {
                    to = to.addSecs(3600);
                }
                if (to < beginDt || to <= from || !to.isValid() || !from.isValid()) {
                    reader.skipCurrentElement();
                    continue;
                }
                auto fc = parseForecastElement(reader);
                for (int i = 0; i < from.secsTo(to); i += 3600) {
                    fc.setTile(tile);
                    fc.setDateTime(from.addSecs(i));
                    fc.setRange(range);
                    result.push_back(fc);
                }
                continue;
            }
            // unknown element
            reader.skipCurrentElement();
        } else {
            reader.readNext();
        }
    }

    return result;
}

// Icon mapping: https://api.met.no/weatherapi/weathericon/1.1/documentation
struct symbol_map_t {
    uint8_t id;
    WeatherForecast::SymbolType type;
};

static const symbol_map_t symbol_map[] = {
    {  1, WeatherForecast::Clear }, // 1 Sun
    {  2, WeatherForecast::Clear | WeatherForecast::LightClouds }, // 2 LightCloud
    {  3, WeatherForecast::Clear | WeatherForecast::Clouds }, // 3 PartlyCloud
    {  4, WeatherForecast::Clouds }, // 4 Cloud
    {  5, WeatherForecast::Clear | WeatherForecast::LightRain }, // 5 LightRainSun
    {  6, WeatherForecast::Clear | WeatherForecast::LightRain | WeatherForecast::ThunderStorm }, // 6 LightRainThunderSun
    {  7, WeatherForecast::Clear | WeatherForecast::Hail }, // 7 SleetSun
    {  8, WeatherForecast::Clear | WeatherForecast::Snow }, // 8 SnowSun
    {  9, WeatherForecast::LightRain }, // 9 LightRain
    { 10, WeatherForecast::Rain }, // 10 Rain
    { 11, WeatherForecast::Rain | WeatherForecast::ThunderStorm }, // 11 RainThunder
    { 12, WeatherForecast::Hail }, // 12 Sleet
    { 13, WeatherForecast::Snow }, // 13 Snow
    { 14, WeatherForecast::Snow | WeatherForecast::ThunderStorm }, // 14 SnowThunder
    { 15, WeatherForecast::Fog }, // 15 Fog
    { 20, WeatherForecast::Clear | WeatherForecast::Hail | WeatherForecast::ThunderStorm }, // 20 SleetSunThunder
    { 21, WeatherForecast::Clear | WeatherForecast::Snow | WeatherForecast::ThunderStorm }, // 21 SnowSunThunder
    { 22, WeatherForecast::LightRain | WeatherForecast::ThunderStorm }, // 22 LightRainThunder
    { 23, WeatherForecast::Hail | WeatherForecast::ThunderStorm }, // 23 SleetThunder
    { 24, WeatherForecast::Clear | WeatherForecast::LightRain | WeatherForecast::ThunderStorm }, // 24 DrizzleThunderSun
    { 25, WeatherForecast::Clear | WeatherForecast::Rain | WeatherForecast::ThunderStorm }, // 25 RainThunderSun
    { 26, WeatherForecast::Clear | WeatherForecast::Hail | WeatherForecast::ThunderStorm }, // 26 LightSleetThunderSun
    { 27, WeatherForecast::Clear | WeatherForecast::Hail | WeatherForecast::ThunderStorm }, // 27 HeavySleetThunderSun
    { 28, WeatherForecast::Clear |  WeatherForecast::LightSnow | WeatherForecast::ThunderStorm }, // 28 LightSnowThunderSun
    { 29, WeatherForecast::Clear | WeatherForecast::Snow | WeatherForecast::ThunderStorm }, // 29 HeavySnowThunderSun
    { 30, WeatherForecast::LightRain | WeatherForecast::ThunderStorm }, // 30 DrizzleThunder
    { 31, WeatherForecast::Hail | WeatherForecast::ThunderStorm }, // 31 LightSleetThunder
    { 32, WeatherForecast::Hail | WeatherForecast::ThunderStorm }, // 32 HeavySleetThunder
    { 33, WeatherForecast::LightSnow | WeatherForecast::ThunderStorm }, // 33 LightSnowThunder
    { 34, WeatherForecast::Snow | WeatherForecast::ThunderStorm }, // 34 HeavySnowThunder
    { 40, WeatherForecast::Clear | WeatherForecast::LightRain }, // 40 DrizzleSun
    { 41, WeatherForecast::Clear | WeatherForecast::Rain}, // 41 RainSun
    { 42, WeatherForecast::Clear | WeatherForecast::Hail }, // 42 LightSleetSun
    { 43, WeatherForecast::Clear | WeatherForecast::Hail }, // 43 HeavySleetSun
    { 44, WeatherForecast::Clear | WeatherForecast::LightSnow }, // 44 LightSnowSun
    { 45, WeatherForecast::Clear | WeatherForecast::Snow }, // 45 HeavysnowSun
    { 46, WeatherForecast::LightRain }, // 46 Drizzle
    { 47, WeatherForecast::Hail }, // 47 LightSleet
    { 48, WeatherForecast::Hail }, // 48 HeavySleet
    { 49, WeatherForecast::LightSnow }, // 49 LightSnow
    { 50, WeatherForecast::Snow } // 50 HeavySnow
};

WeatherForecast WeatherForecastManager::parseForecastElement(QXmlStreamReader &reader) const
{
    WeatherForecast fc;
    while (!reader.atEnd()) {
        switch (reader.tokenType()) {
            case QXmlStreamReader::StartElement:
                if (reader.name() == QLatin1String("temperature")) {
                    const auto t = reader.attributes().value(QLatin1String("value")).toFloat();
                    fc.setMinimumTemperature(t);
                    fc.setMaximumTemperature(t);
                } else if (reader.name() == QLatin1String("minTemperature")) {
                    fc.setMinimumTemperature(reader.attributes().value(QLatin1String("value")).toFloat());
                } else if (reader.name() == QLatin1String("maxTemperature")) {
                    fc.setMaximumTemperature(reader.attributes().value(QLatin1String("value")).toFloat());
                } else if (reader.name() == QLatin1String("windSpeed")) {
                    fc.setWindSpeed(reader.attributes().value(QLatin1String("mps")).toFloat());
                } else if (reader.name() == QLatin1String("symbol")) {
                    auto symId = reader.attributes().value(QLatin1String("number")).toInt();
                    if (symId > 100) {
                        symId -= 100; // map polar night symbols
                    }
                    const auto it = std::lower_bound(std::begin(symbol_map), std::end(symbol_map), symId, [](symbol_map_t lhs, uint8_t rhs) {
                        return lhs.id < rhs;
                    });
                    if (it != std::end(symbol_map) && (*it).id == symId) {
                        fc.setSymbolType((*it).type);
                    }
                } else if (reader.name() == QLatin1String("precipitation")) {
                    fc.setPrecipitation(reader.attributes().value(QLatin1String("value")).toFloat());
                }
                break;
            case QXmlStreamReader::EndElement:
                if (reader.name() == QLatin1String("time")) {
                    return fc;
                }
                break;
            default:
                break;
        }
        reader.readNext();
    }

    return fc;
}

QDateTime WeatherForecastManager::maximumForecastTime(const QDate &today) const
{
    return QDateTime(today.addDays(9), QTime(0, 0));
}

void WeatherForecastManager::setTestModeEnabled(bool testMode)
{
    m_testMode = testMode;
}

void WeatherForecastManager::scheduleUpdate()
{
    if (m_updateTimer.isActive()) {
        return;
    }

    // see §Updates on https://api.met.no/conditions_service.html
    m_updateTimer.setInterval(std::chrono::hours(2) + std::chrono::minutes(QTime::currentTime().msec() % 30));
    qDebug() << "Next weather update:" << m_updateTimer.interval();
    m_updateTimer.start();
}

void WeatherForecastManager::updateAll()
{
    for (const auto tile : m_monitoredTiles) {
        fetchTile(tile);
    }
    purgeCache();
    scheduleUpdate();
}

void WeatherForecastManager::purgeCache()
{
    const auto basePath = QString(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/weather/"));
    const auto cutoffDate = QDateTime::currentDateTimeUtc().addDays(-9);

    QDirIterator it(basePath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Writable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile() && it.fileInfo().lastModified() < cutoffDate) {
            qDebug() << "Purging old weather data:" << it.filePath();
            QFile::remove(it.filePath());
        } else if (it.fileInfo().isDir() && QDir(it.filePath()).isEmpty()) {
            qDebug() << "Purging old weather cache folder:" << it.filePath();
            QDir().rmdir(it.filePath());
        }
    }
}

#include "moc_weatherforecastmanager.cpp"
