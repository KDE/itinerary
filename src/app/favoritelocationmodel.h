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

#ifndef FAVORITELOCATIONMODEL_H
#define FAVORITELOCATIONMODEL_H

#include <QAbstractListModel>
#include <QExplicitlySharedDataPointer>

#include <vector>

class QJsonArray;
class QJsonObject;

class FavoriteLocationPrivate;

/** Favorite location. */
class FavoriteLocation
{
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(float latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(float longitude READ longitude WRITE setLongitude)

public:
    FavoriteLocation();
    FavoriteLocation(const FavoriteLocation&);
    FavoriteLocation(FavoriteLocation&&);
    ~FavoriteLocation();
    FavoriteLocation& operator=(const FavoriteLocation&);

    /** Name set and coordinate valid. */
    bool isValid() const;

    QString name() const;
    void setName(const QString &name);
    float latitude() const;
    void setLatitude(float lat);
    float longitude() const;
    void setLongitude(float lon);

    static FavoriteLocation fromJson(const QJsonObject &obj);
    static std::vector<FavoriteLocation> fromJson(const QJsonArray &array);
    static QJsonArray toJson(const std::vector<FavoriteLocation> &locs);
    static QJsonObject toJson(const FavoriteLocation &loc);
private:
    QExplicitlySharedDataPointer<FavoriteLocationPrivate> d;
};

/** Favorite location management for transfer elements. */
class FavoriteLocationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        LatitudeRole = Qt::UserRole,
        LongitudeRole,
    };
    Q_ENUM(Role)

    explicit FavoriteLocationModel(QObject *parent = nullptr);
    ~FavoriteLocationModel();

    /** Appends a new dummy location. */
    Q_INVOKABLE void appendNewLocation();
    /** Removes location at index @row. */
    Q_INVOKABLE void removeLocation(int row);

    /** All favorite locations. */
    const std::vector<FavoriteLocation>& favoriteLocations() const;
    /** Set favorite locations to @p locs.
     *  Used for importing.
     */
    void setFavoriteLocations(std::vector<FavoriteLocation> &&locs);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void saveLocations() const;

    std::vector<FavoriteLocation> m_locations;
};

#endif // FAVORITELOCATIONMODEL_H
