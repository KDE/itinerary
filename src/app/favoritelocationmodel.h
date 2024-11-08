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
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude)

public:
    FavoriteLocation();
    FavoriteLocation(const FavoriteLocation &);
    FavoriteLocation(FavoriteLocation &&);
    ~FavoriteLocation();
    FavoriteLocation &operator=(const FavoriteLocation &);

    /** Name set and coordinate valid. */
    [[nodiscard]] bool isValid() const;

    [[nodiscard]] QString name() const;
    void setName(const QString &name);
    [[nodiscard]] double latitude() const;
    void setLatitude(double lat);
    [[nodiscard]] double longitude() const;
    void setLongitude(double lon);

    [[nodiscard]] static FavoriteLocation fromJson(const QJsonObject &obj);
    [[nodiscard]] static std::vector<FavoriteLocation> fromJson(const QJsonArray &array);
    [[nodiscard]] static QJsonArray toJson(const std::vector<FavoriteLocation> &locs);
    [[nodiscard]] static QJsonObject toJson(const FavoriteLocation &loc);

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
    ~FavoriteLocationModel() override;

    /** Appends a new dummy location. */
    Q_INVOKABLE void appendNewLocation();
    /** Appends the given location if there is none yet that matches its position. */
    void appendLocationIfMissing(FavoriteLocation &&loc);
    /** Removes location at index @row. */
    Q_INVOKABLE void removeLocation(int row);

    /** All favorite locations. */
    [[nodiscard]] const std::vector<FavoriteLocation> &favoriteLocations() const;
    /** Set favorite locations to @p locs.
     *  Used for importing.
     */
    void setFavoriteLocations(std::vector<FavoriteLocation> &&locs);

    /** Export to GPX. */
    Q_INVOKABLE void exportToGpx(const QString &filePath) const;
    /** Import from GPX. */
    Q_INVOKABLE void importFromGpx(const QString &filePath);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    void saveLocations() const;

    std::vector<FavoriteLocation> m_locations;
};

Q_DECLARE_METATYPE(FavoriteLocation)

#endif // FAVORITELOCATIONMODEL_H
