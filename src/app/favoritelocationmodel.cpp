/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "favoritelocationmodel.h"

#include "gpxexport.h"
#include "json.h"
#include "jsonio.h"
#include "logging.h"

#include "gpx/gpxreader.h"

#include <KItinerary/LocationUtil>

#include <KLocalizedString>

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include <cmath>

class FavoriteLocationPrivate : public QSharedData
{
public:
    QString name;
    float latitude = NAN;
    float longitude = NAN;
};

FavoriteLocation::FavoriteLocation()
    : d(new FavoriteLocationPrivate)
{
}

FavoriteLocation::FavoriteLocation(const FavoriteLocation&) = default;
FavoriteLocation::FavoriteLocation(FavoriteLocation &&) = default;
FavoriteLocation::~FavoriteLocation() = default;
FavoriteLocation& FavoriteLocation::operator=(const FavoriteLocation&) = default;

bool FavoriteLocation::isValid() const
{
    return !d->name.isEmpty() && !std::isnan(d->latitude) && !std::isnan(d->longitude);
}

QString FavoriteLocation::name() const
{
    return d->name;
}

void FavoriteLocation::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

float FavoriteLocation::latitude() const
{
    return d->latitude;
}

void FavoriteLocation::setLatitude(float lat)
{
    d.detach();
    d->latitude = lat;
}

float FavoriteLocation::longitude() const
{
    return d->longitude;
}

void FavoriteLocation::setLongitude(float lon)
{
    d.detach();
    d->longitude = lon;
}

FavoriteLocation FavoriteLocation::fromJson(const QJsonObject& obj)
{
    return Json::fromJson<FavoriteLocation>(obj);
}

std::vector<FavoriteLocation> FavoriteLocation::fromJson(const QJsonArray &array)
{
    return Json::fromJson<FavoriteLocation>(array);
}

QJsonObject FavoriteLocation::toJson(const FavoriteLocation& loc)
{
    return Json::toJson(loc);
}

QJsonArray FavoriteLocation::toJson(const std::vector<FavoriteLocation> &locs)
{
    return Json::toJson(locs);
}


static QString favoriteLocationPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/favorite-locations/");
}

FavoriteLocationModel::FavoriteLocationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // load existing locations
    QFile f(favoriteLocationPath() + QLatin1StringView("locations.json"));
    if (f.open(QFile::ReadOnly)) { // error is fine, file might not exist yet
        const auto val = JsonIO::read(f.readAll());
        beginResetModel();
        m_locations = FavoriteLocation::fromJson(val.toArray());
        endResetModel();
    }

    // migrate old home configuration
    if (m_locations.empty()) {
        FavoriteLocation home;
        QSettings settings;
        settings.beginGroup(QStringLiteral("HomeLocation"));
        home.setLatitude(settings.value(QStringLiteral("Latitude"), NAN).toFloat());
        home.setLongitude(settings.value(QStringLiteral("Longitude"), NAN).toFloat());
        home.setName(i18n("Home"));
        if (home.isValid()) {
            beginInsertRows({}, 0, 0);
            m_locations.push_back(home);
            endInsertRows();
            saveLocations();
        }
        settings.endGroup();
        settings.remove(QStringLiteral("HomeLocation"));
    }
}

FavoriteLocationModel::~FavoriteLocationModel() = default;

void FavoriteLocationModel::appendNewLocation()
{
    beginInsertRows({}, rowCount(), rowCount());
    FavoriteLocation loc;
    switch (rowCount()) {
        case 0:
            loc.setName(i18n("Home"));
            break;
        case 1:
            loc.setName(i18n("Work"));
            break;
        default:
            loc.setName(i18n("Location %1", rowCount() + 1));
            break;
    }
    m_locations.push_back(loc);
    endInsertRows();
    saveLocations();
}

void FavoriteLocationModel::appendLocationIfMissing(FavoriteLocation &&loc)
{
    for (const auto &l : m_locations) {
        if (KItinerary::LocationUtil::distance(l.latitude(), l.longitude(), loc.latitude(), loc.longitude()) < 10) {
            qCDebug(Log) << "Not importing" << loc.name() << "due to" << l.name() << "close by";
            return;
        }
    }
    beginInsertRows({}, rowCount(), rowCount());
    m_locations.push_back(std::move(loc));
    endInsertRows();
}

void FavoriteLocationModel::removeLocation(int row)
{
    beginRemoveRows({}, row, row);
    m_locations.erase(m_locations.begin() + row);
    endRemoveRows();
    saveLocations();
}

const std::vector<FavoriteLocation>& FavoriteLocationModel::favoriteLocations() const
{
    return m_locations;
}

void FavoriteLocationModel::setFavoriteLocations(std::vector<FavoriteLocation> &&locs)
{
    beginResetModel();
    m_locations = std::move(locs);
    saveLocations();
    endResetModel();
}

int FavoriteLocationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_locations.size();
}

QVariant FavoriteLocationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &loc = m_locations[index.row()];
    switch (role) {
        case Qt::DisplayRole:
            return loc.name();
        case FavoriteLocationModel::LatitudeRole:
            return loc.latitude();
        case FavoriteLocationModel::LongitudeRole:
            return loc.longitude();
        case FavoriteLocationModel::FavoriteLocationRole:
            return QVariant::fromValue(loc);
    }

    return {};
}

bool FavoriteLocationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    auto &loc = m_locations[index.row()];
    switch (role) {
        case Qt::DisplayRole:
            loc.setName(value.toString());
            Q_EMIT dataChanged(index, index);
            saveLocations();
            return true;
        case FavoriteLocationModel::LatitudeRole:
            loc.setLatitude(value.toFloat());
            Q_EMIT dataChanged(index, index);
            saveLocations();
            return true;
        case FavoriteLocationModel::LongitudeRole:
            loc.setLongitude(value.toFloat());
            Q_EMIT dataChanged(index, index);
            saveLocations();
            return true;
    }

    return false;
}

QHash<int, QByteArray> FavoriteLocationModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(FavoriteLocationModel::LatitudeRole, "latitude");
    r.insert(FavoriteLocationModel::LongitudeRole, "longitude");
    r.insert(FavoriteLocationModel::FavoriteLocationRole, "favoriteLocation");
    return r;
}

void FavoriteLocationModel::saveLocations() const
{
    const auto basePath = favoriteLocationPath();
    QDir().mkpath(basePath);
    QFile f(basePath + QLatin1StringView("locations.json"));
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "Failed to save favorite locations:" << f.errorString() << f.fileName();
        return;
    }

    f.write(JsonIO::write(FavoriteLocation::toJson(m_locations)));
}

void FavoriteLocationModel::exportToGpx(const QString &filePath) const
{
    if (filePath.isEmpty()) {
        return;
    }

    QFile f(QUrl(filePath).isLocalFile() ? QUrl(filePath).toLocalFile() : filePath);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << f.errorString() << f.fileName();
        return;
    }
    GpxExport exporter(&f);
    for (const auto &fav : m_locations) {
        exporter.writeFavoriteLocation(fav);
    }
}

void FavoriteLocationModel::importFromGpx(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    QFile f(QUrl(filePath).isLocalFile() ? QUrl(filePath).toLocalFile() : filePath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.errorString() << f.fileName();
        return;
    }

    Gpx::Reader reader(&f);
    while (reader.readNextStartElement()) {
        if (reader.isRootElement()) {
            continue;
        }
        if (reader.isWaypointStart()) {
            FavoriteLocation loc;
            loc.setLatitude(reader.latitude());
            loc.setLongitude(reader.longitude());

            QString name, type;
            while (reader.readNext()) {
                if (reader.isWaypointEnd()) {
                    break;
                }
                if (reader.isGpxName()) {
                    name = reader.gpxName();
                } else if (reader.isGpxType()) {
                    type = reader.gpxType();
                }
            }

            if (name.isEmpty()) {
                continue;
            }
            if (!type.isEmpty()) {
                loc.setName(type + QLatin1Char('/') + name);
            } else {
                loc.setName(name);
            }

            appendLocationIfMissing(std::move(loc));
            continue;
        }
        reader.skipCurrentElement();
    }

    saveLocations();
}

#include "moc_favoritelocationmodel.cpp"
