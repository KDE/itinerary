// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef TRIPGROUPMAPMODEL_H
#define TRIPGROUPMAPMODEL_H

#include "livedatamanager.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"

#include <KPublicTransport/Journey>
#include <KPublicTransport/Location>

#include <QColor>
#include <QObject>

struct MapPathEntry {
    Q_GADGET
    Q_PROPERTY(KPublicTransport::JourneySection journeySection MEMBER journeySection CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
    Q_PROPERTY(QColor textColor MEMBER textColor CONSTANT)
    Q_PROPERTY(double width MEMBER width CONSTANT)
    Q_PROPERTY(bool showStart MEMBER showStart CONSTANT)
    Q_PROPERTY(bool showEnd MEMBER showEnd CONSTANT)

public:
    KPublicTransport::JourneySection journeySection;
    QColor color;
    QColor textColor;
    double width = 1.0;
    bool showStart = true;
    bool showEnd = true;
};

struct MapPointEntry {
    Q_GADGET
    Q_PROPERTY(KPublicTransport::Location location MEMBER location CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
    Q_PROPERTY(QColor textColor MEMBER textColor CONSTANT)
    Q_PROPERTY(QString iconName MEMBER iconName CONSTANT)

public:
    KPublicTransport::Location location;
    QColor color;
    QColor textColor;
    QString iconName;
};

class TripGroupMapModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString tripGroupId MEMBER m_tripGroupId NOTIFY tripGroupIdChanged FINAL)
    Q_PROPERTY(TripGroupManager *tripGroupManager MEMBER m_tripGroupMgr NOTIFY setupChanged FINAL)
    Q_PROPERTY(LiveDataManager *liveDataManager MEMBER m_liveDataMgr NOTIFY setupChanged FINAL)
    Q_PROPERTY(TransferManager *transferManager MEMBER m_transferMgr NOTIFY setupChanged FINAL)

    Q_PROPERTY(QList<MapPathEntry> journeySections READ journeySections NOTIFY contentChanged FINAL)
    Q_PROPERTY(QList<MapPointEntry> points READ points NOTIFY contentChanged FINAL)
    Q_PROPERTY(QRectF boundingBox READ boundingBox NOTIFY contentChanged FINAL)

public:
    explicit TripGroupMapModel(QObject *parent = nullptr);
    ~TripGroupMapModel();

    [[nodiscard]] QList<MapPathEntry> journeySections() const;
    [[nodiscard]] QList<MapPointEntry> points() const;
    [[nodiscard]] QRectF boundingBox() const;

Q_SIGNALS:
    void tripGroupIdChanged();
    void contentChanged();
    void setupChanged();

private:
    void recompute();
    void expandJourney(const KPublicTransport::Journey &jny);
    void expandJourneySection(KPublicTransport::JourneySection jnySec);

    QString m_tripGroupId;
    QList<MapPathEntry> m_journeySections;
    QList<MapPointEntry> m_points;
    QRectF m_boundingBox;

    TripGroupManager *m_tripGroupMgr = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
};

#endif
