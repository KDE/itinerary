/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
        FavoriteLocationRole,
    };
    Q_ENUM(Role)

    explicit FavoriteLocationModel(QObject *parent = nullptr);
    ~FavoriteLocationModel();

    /** Appends a new dummy location. */
    Q_INVOKABLE void appendNewLocation();
    /** Appends the given location if there is none yet that matches its position. */
    void appendLocationIfMissing(FavoriteLocation &&loc);
    /** Removes location at index @row. */
    Q_INVOKABLE void removeLocation(int row);

    /** All favorite locations. */
    const std::vector<FavoriteLocation>& favoriteLocations() const;
    /** Set favorite locations to @p locs.
     *  Used for importing.
     */
    void setFavoriteLocations(std::vector<FavoriteLocation> &&locs);

    /** Export to GPX. */
    Q_INVOKABLE void exportToGpx(const QString &filePath) const;
    /** Import from GPX. */
    Q_INVOKABLE void importFromGpx(const QString &filePath);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void saveLocations() const;

    std::vector<FavoriteLocation> m_locations;
};

Q_DECLARE_METATYPE(FavoriteLocation)

#endif // FAVORITELOCATIONMODEL_H
