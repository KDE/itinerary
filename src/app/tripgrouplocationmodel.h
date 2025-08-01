// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef TRIPGROUPLOCATIONMODEL_H
#define TRIPGROUPLOCATIONMODEL_H

#include "tripgroupmanager.h"

#include <KPublicTransport/Location>

#include <QAbstractListModel>
#include <QDateTime>
#include <qqmlintegration.h>

#include <vector>

/** All locations visitied during a trip group, for pre-filling the stop picker.
 *  This excludes anything without a geo position, as that can otherwise result in
 *  leaking out internal information via geocoding.
 */
class TripGroupLocationModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString tripGroupId MEMBER m_tripGroupId NOTIFY setupChanged)
    Q_PROPERTY(TripGroupManager *tripGroupManager MEMBER m_tripGroupMgr WRITE setTripGroupManager NOTIFY setupChanged)

public:
    explicit TripGroupLocationModel(QObject *parent = nullptr);
    ~TripGroupLocationModel();

    void setTripGroupManager(TripGroupManager *tripGroupMgr);

    // must match KPublicTransport::LocationHistoryModel for use in QConcatenateTableProxyModel
    enum Role {
        LocationRole = Qt::UserRole,
        LocationNameRole,
        LastUsedRole,
        UseCountRole,
        IsRemovableRole,
    };
    Q_ENUM(Role)

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void setupChanged();
    void locationsChanged();

private:
    struct Entry {
        KPublicTransport::Location location;
        QDateTime lastUse;
        int useCount = 1;
    };

    void populate();
    void addEntry(Entry &&entry);

    std::vector<Entry> m_locations;

    QString m_tripGroupId;
    TripGroupManager *m_tripGroupMgr = nullptr;
};

#endif
