// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "matrixbeacon.h"
#include "matrixmanager.h"

#include <Quotient/room.h>
#include <Quotient/csapi/room_state.h>
#include <Quotient/events/stateevent.h>

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
    Quotient::StateEvent ev(QLatin1String("org.matrix.msc3672.beacon_info"), m_connection->userId(), QJsonObject{
        {QLatin1String("description"), description},
        {QLatin1String("live"), true},
        {QLatin1String("org.matrix.msc3488.ts"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()},
        {QLatin1String("timeout"), 300000},
        {QLatin1String("org.matrix.msc3488.asset"), QJsonObject{
            {QLatin1String("type"), QLatin1String("m.self")},
        }},
    });
    auto job = room->setState(ev);
    connect(job, &Quotient::SetRoomStateWithKeyJob::result, this, [job, this]() {
        qDebug() << job->errorString() << job->jsonData();
        if (job->error() == Quotient::BaseJob::NoError) {
            setBeaconInfoId(job->jsonData().value(QLatin1String("event_id")).toString());
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

    Quotient::StateEvent ev(QLatin1String("org.matrix.msc3672.beacon_info"), m_connection->userId(), QJsonObject{
        {QLatin1String("live"), false},
        {QLatin1String("org.matrix.msc3488.ts"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()},
        {QLatin1String("timeout"), 300000},
        {QLatin1String("org.matrix.msc3488.asset"), QJsonObject{
            {QLatin1String("type"), QLatin1String("m.self")},
        }},
    });
    room->setState(ev);
}

void MatrixBeacon::updateLocation(float latitude, float longitude)
{
    if (!m_connection || m_roomId.isEmpty() || !isActive()) {
        return;
    }

    auto room = m_connection->room(m_roomId);
    if (!room) {
        qWarning() << "invalid room" << m_roomId;
        return;
    }

    QJsonObject content{
        {QLatin1String("msgtype"), QLatin1String("org.matrix.msc3672.beacon")},
        {QLatin1String("org.matrix.msc3488.ts"), QDateTime::currentDateTime().toMSecsSinceEpoch()},
        {QLatin1String("org.matrix.msc3488.location"), QJsonObject {
            {QLatin1String("uri"), QLatin1String("geo:%1,%2").arg(QString::number(latitude), QString::number(longitude))},
            // {QLatin1String("description"), m_description} TODO?
        }},
        {QLatin1String("m.relates_to"), QJsonObject{
            {QLatin1String("rel_type"), QLatin1String("m.reference")},
            {QLatin1String("event_id"), m_beaconInfoId},
        }},
    };
    room->postJson(QLatin1String("org.matrix.msc3672.beacon"), content);
}

void MatrixBeacon::setBeaconInfoId(const QString& id)
{
    if (m_beaconInfoId == id) {
        return;
    }

    m_beaconInfoId = id;
    Q_EMIT activeChanged();
}
