// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef MATRIXBEACON_H
#define MATRIXBEACON_H

#include <QObject>

#include <Quotient/connection.h>

#include <cmath>

/** Live location sharing.
 *  @see https://github.com/matrix-org/matrix-spec-proposals/pull/3489
 */
class MatrixBeacon : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QString roomId READ roomId WRITE setRoomId NOTIFY roomIdChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY activeChanged)

public:
    explicit MatrixBeacon(QObject *parent = nullptr);
    ~MatrixBeacon();

    Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    QString roomId() const;
    void setRoomId(const QString &roomId);

    /** Returns true if the beacon is currently live. */
    bool isActive() const;

public Q_SLOTS:
    void start(const QString &description);
    void stop();
    /**
     * @param latitude/longitude in degree
     * @param heading in degree
     * @param speed in km/s
     * @param altitude in meter
     */
    void updateLocation(double latitude, double longitude, float heading = NAN, float speed = NAN, float altitude = NAN);

Q_SIGNALS:
    void connectionChanged();
    void roomIdChanged();
    void activeChanged();

private:
    void setBeaconInfoId(const QString &id);

    Quotient::Connection *m_connection = nullptr;
    QString m_roomId;
    QString m_beaconInfoId;
    double m_latitude = NAN;
    double m_longitude = NAN;
};

#endif // MATRIXBEACON_H
