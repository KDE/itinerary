/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "downloadjob.h"
#include "logging.h"

#include <KLocalizedString>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

DownloadJob::DownloadJob(QUrl url, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
{
    url.setScheme(QLatin1String("https"));
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto reply = nam->get(req);
    reply->setParent(this);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCDebug(Log) << reply->url() << reply->errorString();
            m_errorMessage = i18n("Download failed: %1", reply->errorString());
        } else {
            m_data = reply->readAll();
        }
        Q_EMIT finished();
    });
}

DownloadJob::~DownloadJob() = default;

bool DownloadJob::hasError() const
{
    return !m_errorMessage.isEmpty();
}

QByteArray DownloadJob::data() const
{
    return m_data;
}

QString DownloadJob::errorMessage() const
{
    return m_errorMessage;
}
