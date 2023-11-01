/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DEUTSCHEBAHNNEXTCHECKIN_H
#define DEUTSCHEBAHNNEXTCHECKIN_H

#include "reservationmanager.h"

#include <QObject>

class QNetworkAccessManager;

/** Deutsche Bahn (DB) Komfort Check-In (KCI) for the DB Next API. */
class DeutscheBahnNextCheckIn : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ReservationManager* reservationManager MEMBER m_resMgr NOTIFY reservationManagerChanged)
public:
    explicit DeutscheBahnNextCheckIn(QObject *parent = nullptr);
    ~DeutscheBahnNextCheckIn();

    Q_INVOKABLE bool checkInAvailable(const QVariant &res) const;
    Q_INVOKABLE bool checkInEnabled(const QVariant &res) const;

    // TODO multi-traveler check-in?
    Q_INVOKABLE void checkIn(const QString &resId);

    static void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory);

Q_SIGNALS:
    void reservationManagerChanged();

private:
    void doAuthenticate(const QString &resId);

    static std::function<QNetworkAccessManager*()> s_namFactory;
    ReservationManager *m_resMgr = nullptr;
};

#endif
