// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "matrixbeacon.h"
#include "matrixmanager.h"

#include <Quotient/csapi/room_state.h>
#include <Quotient/events/stateevent.h>
#include <Quotient/room.h>

#include <QDateTime>
#include <QJsonObject>

MatrixBeacon::MatrixBeacon(QObject *parent)
    : QObject(parent)
{
}

MatrixBeacon::~MatrixBeacon()
{
    stop();
}

Quotient::Connection *MatrixBeacon::connection() const
{
    return m_connection;
}

void MatrixBeacon::setConnection(Quotient::Connection *connection)
{
    if (m_connection == connection) {
        return;
    }

    m_connection = connection;
    Q_EMIT connectionChanged();
}

QString MatrixBeacon::roomId() const
{
    return m_roomId;
}

void MatrixBeacon::setRoomId(const QString &roomId)
{
    if (m_roomId == roomId) {
        return;
    }

    m_roomId = roomId;
    Q_EMIT roomIdChanged();
}

bool MatrixBeacon::isActive() const
{
    return !m_beaconInfoId.isEmpty();
}

void MatrixBeacon::start(const QString &description)
{
    if (!m_connection || m_roomId.isEmpty()) {
        return;
    }

    auto room = m_connection->room(m_roomId);
    if (!room) {
        qWarning() << "invalid room" << m_roomId;
        return;
    }

    // TODO beacon id according to MSC3757
    Quotient::StateEvent ev(QLatin1StringView("org.matrix.msc3672.beacon_info"),
                            m_connection->userId(),
                            QJsonObject{
                                {QLatin1StringView("description"), description},
                                {QLatin1StringView("live"), true},
                                {QLatin1StringView("org.matrix.msc3488.ts"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()},
                                {QLatin1StringView("timeout"), 300000},
                                {QLatin1StringView("org.matrix.msc3488.asset"),
                                 QJsonObject{
                                     {QLatin1StringView("type"), QLatin1StringView("m.self")},
                                 }},
                            });
    auto job = room->setState(ev);
    connect(job, &Quotient::SetRoomStateWithKeyJob::result, this, [job, this]() {
        qDebug() << job->errorString() << job->jsonData();
        if (job->error() == Quotient::BaseJob::NoError) {
            setBeaconInfoId(job->jsonData().value(QLatin1StringView("event_id")).toString());
            updateLocation(m_latitude, m_longitude);
        } else {
            qWarning() << job->errorString();
        }
    });
}

void MatrixBeacon::stop()
{
    setBeaconInfoId(QString());
    if (!m_connection || m_roomId.isEmpty()) {
        return;
    }

    auto room = m_connection->room(m_roomId);
    if (!room) {
        qWarning() << "invalid room" << m_roomId;
        return;
    }

    Quotient::StateEvent ev(QLatin1StringView("org.matrix.msc3672.beacon_info"),
                            m_connection->userId(),
                            QJsonObject{
                                {QLatin1StringView("live"), false},
                                {QLatin1StringView("org.matrix.msc3488.ts"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()},
                                {QLatin1StringView("timeout"), 300000},
                                {QLatin1StringView("org.matrix.msc3488.asset"),
                                 QJsonObject{
                                     {QLatin1StringView("type"), QLatin1StringView("m.self")},
                                 }},
                            });
    room->setState(ev);
}

static QString geoUri(double latitude, double longitude, float altitude)
{
    QString uri = QLatin1StringView("geo:") + QString::number(latitude) + QLatin1Char(',') + QString::number(longitude);
    if (!std::isnan(altitude)) {
        uri += QLatin1Char(',') + QString::number(altitude);
    }
    return uri;
}

void MatrixBeacon::updateLocation(double latitude, double longitude, float heading, float speed, float altitude)
{
    m_latitude = latitude;
    m_longitude = longitude;
    if (!m_connection || m_roomId.isEmpty() || !isActive() || std::isnan(m_latitude) || std::isnan(m_longitude)) {
        return;
    }

    auto room = m_connection->room(m_roomId);
    if (!room) {
        qWarning() << "invalid room" << m_roomId;
        return;
    }

    QJsonObject location({{QLatin1StringView("uri"), geoUri(m_latitude, m_longitude, altitude)}});
    if (!std::isnan(heading)) {
        location.insert(QLatin1StringView("org.kde.itinerary.heading"), QString::number(heading));
    }
    if (!std::isnan(speed)) {
        location.insert(QLatin1StringView("org.kde.itinerary.speed"), QString::number(speed));
    }

    QJsonObject content{
        {QLatin1StringView("msgtype"), QLatin1StringView("org.matrix.msc3672.beacon")},
        {QLatin1StringView("org.matrix.msc3488.ts"), QDateTime::currentDateTime().toMSecsSinceEpoch()},
        {QLatin1StringView("org.matrix.msc3488.location"), location},
        {QLatin1StringView("m.relates_to"),
         QJsonObject{
             {QLatin1StringView("rel_type"), QLatin1StringView("m.reference")},
             {QLatin1StringView("event_id"), m_beaconInfoId},
         }},
    };
    room->postJson(QLatin1StringView("org.matrix.msc3672.beacon"), content);
}

void MatrixBeacon::setBeaconInfoId(const QString &id)
{
    if (m_beaconInfoId == id) {
        return;
    }

    m_beaconInfoId = id;
    Q_EMIT activeChanged();
}

#include "moc_matrixbeacon.cpp"
