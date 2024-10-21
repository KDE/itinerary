/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ONLINETICKETRETRIEVALJOB_H
#define ONLINETICKETRETRIEVALJOB_H

#include <QJsonArray>
#include <QObject>

class QIODevice;
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
    [[nodiscard]] QJsonArray result() const;
    [[nodiscard]] QString errorMessage() const;

Q_SIGNALS:
    /** Emitted when the job finished, regardless of success.
     *  Deletion of the job afterwards is consumer responsibility.
     */
    void finished();

private:
    void dbRequestFindOrder(const QVariantMap &arguments);
    static QStringList dbParseKwid(QIODevice *io);
    void dbRequestOrderDetails(const QVariantMap &arguments);

    void setupReply(QNetworkReply *reply);
    void handleReply(QNetworkReply *reply);

    QJsonArray m_result;
    QString m_errorMsg;
    QNetworkAccessManager *m_nam;
    int m_pendingReplies = 0;
};

#endif // ONLINETICKETRETRIEVALJOB_H
