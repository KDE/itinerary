/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DEUTSCHEBAHNCHECKIN_H
#define DEUTSCHEBAHNCHECKIN_H

#include "reservationmanager.h"

#include <QObject>

class QNetworkAccessManager;

/** Deutsche Bahn (DB) Komfort Check-In (KCI) support. */
class DeutscheBahnCheckIn : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ReservationManager* reservationManager MEMBER m_resMgr NOTIFY reservationManagerChanged)
public:
    explicit DeutscheBahnCheckIn(QObject *parent = nullptr);
    ~DeutscheBahnCheckIn();

    Q_INVOKABLE bool checkInAvailable(const QVariant &res) const;
    Q_INVOKABLE bool checkInEnabled(const QVariant &res) const;

    // TODO multi-traveler check-in?
    Q_INVOKABLE void checkIn(const QString &resId);

    static void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory);

Q_SIGNALS:
    void reservationManagerChanged();

private:
    void doCheckIn(const QVariant &res);

    static std::function<QNetworkAccessManager*()> s_namFactory;
    ReservationManager *m_resMgr = nullptr;
};

#endif
