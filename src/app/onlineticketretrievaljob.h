/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ONLINETICKETRETRIEVALJOB_H
#define ONLINETICKETRETRIEVALJOB_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

/** Online ticket retrieval job. */
class OnlineTicketRetrievalJob : public QObject
{
    Q_OBJECT
public:
    /*
     * @param sourceId Identifier of the service the ticket is retrieved from.
     * @param arguments Service-specific arguments needed for retrieving a ticket.
     */
    explicit OnlineTicketRetrievalJob(const QString &sourceId, const QVariantMap &arguments, QNetworkAccessManager *nam, QObject *parent = nullptr);
    ~OnlineTicketRetrievalJob();

    /** Retrieved ticket(s), e.g. as KItinerary::TrainReservation instances. */
    QVector<QVariant> result() const;
    QString errorMessage() const;

Q_SIGNALS:
    /** Emitted when the job finished, regardless of success.
     *  Deletion of the job afterwards is consumer responsibility.
     */
    void finished();

private:
    void handleReply(QNetworkReply *reply);

    QVector<QVariant> m_result;
    QString m_errorMsg;
};

#endif // ONLINETICKETRETRIEVALJOB_H
