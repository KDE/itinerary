/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include <QObject>

class QNetworkAccessManager;
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
    explicit DownloadJob(QUrl url, QNetworkAccessManager *nam, QObject *parent = nullptr);
    ~DownloadJob();

    bool hasError() const;
    QString errorMessage() const;
    QByteArray data() const;

Q_SIGNALS:
    void finished();

private:
    bool handleOnlineTicketRetrievalUrl(const QUrl &url, QNetworkAccessManager *nam);

    QByteArray m_data;
    QString m_errorMessage;
};

#endif
