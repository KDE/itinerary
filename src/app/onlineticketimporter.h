/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ONLINETICKETIMPORTER_H
#define ONLINETICKETIMPORTER_H

#include "reservationmanager.h"

#include <QObject>

class OnlineTicketRetrievalJob;
class ReservationManager;

class QNetworkAccessManager;

/** QML interface for online ticket imports. */
class OnlineTicketImporter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY searchingChanged)
    Q_PROPERTY(ReservationManager* reservationManager MEMBER m_resMgr NOTIFY reservationManagerChanged)

public:
    explicit OnlineTicketImporter(QObject *parent = nullptr);
    ~OnlineTicketImporter();

    bool searching() const;
    QString errorMessage() const;

    Q_INVOKABLE void search(const QString &sourceId, const QVariantMap &arguments);

Q_SIGNALS:
    void searchingChanged();
    void searchSucceeded();
    void searchFailed();
    void reservationManagerChanged();

private:
    void handleRetrievalFinished();

    QNetworkAccessManager* nam();
    static QNetworkAccessManager *m_nam;

    ReservationManager *m_resMgr = nullptr;
    OnlineTicketRetrievalJob *m_currentJob = nullptr;
    QString m_errorMessage;
};

#endif // ONLINETICKETIMPORTER_H
