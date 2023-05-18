// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef MATRIXBEACON_H
#define MATRIXBEACON_H

#include <QObject>

#include <Quotient/connection.h>

/** Live location sharing.
 *  @see https://github.com/matrix-org/matrix-spec-proposals/pull/3489
 */
class MatrixBeacon : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QString roomId READ roomId WRITE setRoomId NOTIFY roomIdChanged)

public:
    explicit MatrixBeacon(QObject *parent = nullptr);
    ~MatrixBeacon();

    Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    QString roomId() const;
    void setRoomId(const QString &roomId);

public Q_SLOTS:
    void start(const QString &description);
    void stop();
    void updateLocation(float latitude, float longitude);

Q_SIGNALS:
    void connectionChanged();
    void roomIdChanged();

private:
    Quotient::Connection *m_connection = nullptr;
    QString m_roomId;
    QString m_beaconInfoId;
};

#endif // MATRIXBEACON_H
