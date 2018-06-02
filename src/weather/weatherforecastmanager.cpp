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

#include "weatherforecastmanager.h"
#include "weatherforecast.h"
#include "weathertile.h"

#include <QCoreApplication>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QVariant>
#include <QXmlStreamReader>

#include <zlib.h>

#include <cmath>

/*
 * ATTENTION!
 * Before touching anything in here, especially regarding the network operations
 * make sure to read and understand https://api.met.no/conditions_service.html!
 */

WeatherForecastManager::WeatherForecastManager(QObject *parent)
    : QObject(parent)
{
}

WeatherForecastManager::~WeatherForecastManager() = default;

void WeatherForecastManager::setAllowNetworkAccess(bool enabled)
{
    m_allowNetwork = enabled;
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

QVariant WeatherForecastManager::forecast(float latitude, float longitude, const QDateTime& dt) const
{
    WeatherTile t{latitude, longitude};
    QFile f(cachePath(t) + QLatin1String("forecast.xml"));
    if (f.exists() && f.open(QFile::ReadOnly)) {
        QXmlStreamReader reader(&f);
        auto forecasts = parseForecast(reader);
        mergeForecasts(forecasts);
        auto it = std::lower_bound(forecasts.begin(), forecasts.end(), dt, [](const WeatherForecast &lhs, const QDateTime &rhs) {
            return lhs.dateTime() < rhs;
        });
        if (it != forecasts.begin()) {
            --it;
        }
        if (it != forecasts.end()) {
            return QVariant::fromValue(*it);
        }

    }
    return {};
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
    url.setPath(QStringLiteral("/weatherapi/locationforecast/1.9/"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("lat"), QString::number(tile.lat / WeatherTile::Size));
    query.addQueryItem(QStringLiteral("lon"), QString::number(tile.lon / WeatherTile::Size));
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
        emit forecastUpdated();
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
}

void WeatherForecastManager::mergeForecasts(std::vector<WeatherForecast>& forecasts) const
{
    std::stable_sort(forecasts.begin(), forecasts.end(), [](const WeatherForecast &lhs, const WeatherForecast &rhs) {
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
                break;
            }
        }
        ++storeIt;
        it = mergeIt;
    }
    forecasts.erase(storeIt, forecasts.end());
}

static void alignToHour(QDateTime &dt)
{
    dt.setTime(QTime(dt.time().hour(), 0, 0, 0));
}

std::vector<WeatherForecast> WeatherForecastManager::parseForecast(QXmlStreamReader &reader) const
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
                if (to == from) {
                    to = to.addSecs(3600);
                }
                if (to < beginDt || to <= from || !to.isValid() || !from.isValid()) {
                    reader.skipCurrentElement();
                    continue;
                }
                auto fc = parseForecastElement(reader);
                for (int i = 0; i < from.secsTo(to); i += 3600) {
                    fc.setDateTime(from.addSecs(i * 3600));
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
static const WeatherForecast::SymbolType symbol_map[] = {
    WeatherForecast::Unknown, // 0
    WeatherForecast::Clear, // 1 Sun
    WeatherForecast::LightClouds, // 2 LightCloud
    WeatherForecast::PartlyCloudy, // 3 PartlyCloud
    WeatherForecast::Clouds, // 4 Cloud
    WeatherForecast::LightRainShowers, // 5 LightRainSun
    WeatherForecast::LightRainShowers, // 6 LightRainThunderSun
    WeatherForecast::Hail, // 7 SleetSun
    WeatherForecast::LightSnowShowers, // 8 SnowSun
    WeatherForecast::LightRain, // 9 LightRain
    WeatherForecast::Rain, // 10 Rain
    WeatherForecast::ThunderStorm, // 11 RainThunder
    WeatherForecast::Hail, // 12 Sleet
    WeatherForecast::Snow, // 13 Snow
    WeatherForecast::Snow, // 14 SnowThunder
    WeatherForecast::Fog, // 15 Fog
    WeatherForecast::Hail, // 20 SleetSunThunder
    WeatherForecast::Unknown, // 21 SnowSunThunder
    WeatherForecast::LightRain, // 22 LightRainThunder
    WeatherForecast::Hail, // 23 SleetThunder
    WeatherForecast::ThunderStormShowers, // 24 DrizzleThunderSun
    WeatherForecast::ThunderStormShowers, // 25 RainThunderSun
    WeatherForecast::ThunderStormShowers, // 26 LightSleetThunderSun
    WeatherForecast::Hail, // 27 HeavySleetThunderSun
    WeatherForecast::LightSnowShowers, // 28 LightSnowThunderSun
    WeatherForecast::Snow, // 29 HeavySnowThunderSun
    WeatherForecast::ThunderStorm, // 30 DrizzleThunder
    WeatherForecast::Hail, // 31 LightSleetThunder
    WeatherForecast::Hail, // 32 HeavySleetThunder
    WeatherForecast::Snow, // 33 LightSnowThunder
    WeatherForecast::Snow, // 34 HeavySnowThunder
    WeatherForecast::LightRainShowers, // 40 DrizzleSun
    WeatherForecast::RainShowers, // 41 RainSun
    WeatherForecast::Hail, // 42 LightSleetSun
    WeatherForecast::Hail, // 43 HeavySleetSun
    WeatherForecast::LightSnowShowers, // 44 LightSnowSun
    WeatherForecast::Snow, // 45 HeavysnowSun
    WeatherForecast::LightRain, // 46 Drizzle
    WeatherForecast::Hail, // 47 LightSleet
    WeatherForecast::Hail, // 48 HeavySleet
    WeatherForecast::LightSnow, // 49 LightSnow
    WeatherForecast::Snow// 50 HeavySnow
};

WeatherForecast WeatherForecastManager::parseForecastElement(QXmlStreamReader &reader) const
{
    WeatherForecast fc;
    while (!reader.atEnd()) {
        switch (reader.tokenType()) {
            case QXmlStreamReader::StartElement:
                if (reader.name() == QLatin1String("temperature")) {
                    fc.setTemperature(reader.attributes().value(QLatin1String("value")).toFloat());
                } else if (reader.name() == QLatin1String("symbol")) {
                    auto symId = reader.attributes().value(QLatin1String("number")).toInt();
                    if (symId > 100) {
                        symId -= 100; // map polar night symbols
                    }
                    if (symId <= 50) {
                        fc.setSymbolType(symbol_map[symId]);
                    }
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

#include "moc_weatherforecastmanager.cpp"
