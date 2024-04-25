/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QUrl;

/** Download content from an online source.
 *  This handles the following cases (or will do so eventually):
 *  - URLs that actually needs to be translated to API calls via the OnlineTicketRetrievalJob.
 *  - ActivityPub data
 *  - regular HTTP GET'ed content processed by the extractor engine
 */
class DownloadJob : public QObject
{
    Q_OBJECT
public:
    explicit DownloadJob(const QUrl &url, QNetworkAccessManager *nam, QObject *parent = nullptr);
    ~DownloadJob();

    [[nodiscard]] bool hasError() const;
    [[nodiscard]] QString errorMessage() const;
    [[nodiscard]] QByteArray data() const;

Q_SIGNALS:
    void finished();

private:
    [[nodiscard]] bool handleOsmUrl(const QUrl &url, QNetworkAccessManager *nam);
    [[nodiscard]] bool handleOnlineTicketRetrievalUrl(const QUrl &url, QNetworkAccessManager *nam);
    [[nodiscard]] QNetworkReply* makeActivityPubRequest(QUrl url, QNetworkAccessManager *nam);
    [[nodiscard]] bool handleActivityPubReply(QNetworkReply *reply);
    [[nodiscard]] QNetworkReply* makeDownloadRequest(QUrl url, QNetworkAccessManager *nam);
    void handleDownloadReply(QNetworkReply *reply);

    QByteArray m_data;
    QString m_errorMessage;
};

#endif
