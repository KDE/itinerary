/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#include "favoritelocationmodel.h"
#include "json.h"

#include <KLocalizedString>

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
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
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/favorite-locations/");
}

FavoriteLocationModel::FavoriteLocationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // load existing locations
    QFile f(favoriteLocationPath() + QLatin1String("locations.json"));
    if (f.open(QFile::ReadOnly)) { // error is fine, file might not exist yet
        const auto doc = QJsonDocument::fromJson(f.readAll());
        beginResetModel();
        m_locations = FavoriteLocation::fromJson(doc.array());
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
            emit dataChanged(index, index);
            saveLocations();
            return true;
        case FavoriteLocationModel::LatitudeRole:
            loc.setLatitude(value.toFloat());
            emit dataChanged(index, index);
            saveLocations();
            return true;
        case FavoriteLocationModel::LongitudeRole:
            loc.setLongitude(value.toFloat());
            emit dataChanged(index, index);
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
    QFile f(basePath + QLatin1String("locations.json"));
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "Failed to save favorite locations:" << f.errorString() << f.fileName();
        return;
    }

    f.write(QJsonDocument(FavoriteLocation::toJson(m_locations)).toJson());
}
