/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "onlineticketimporter.h"
#include "onlineticketretrievaljob.h"
#include "logging.h"

#include <QNetworkAccessManager>
#include <QStandardPaths>

#include <cassert>

QNetworkAccessManager* OnlineTicketImporter::m_nam = nullptr;

OnlineTicketImporter::OnlineTicketImporter(QObject *parent)
    : QObject(parent)
{
}

OnlineTicketImporter::~OnlineTicketImporter() = default;

bool OnlineTicketImporter::searching() const
{
    return m_currentJob;
}

QString OnlineTicketImporter::errorMessage() const
{
    return m_errorMessage;
}

void OnlineTicketImporter::search(const QString &sourceId, const QVariantMap &arguments)
{
    qCDebug(Log) << sourceId << arguments;
    delete m_currentJob;

    m_currentJob = new OnlineTicketRetrievalJob(sourceId, arguments, nam(), this);
    connect(m_currentJob, &OnlineTicketRetrievalJob::finished, this, &OnlineTicketImporter::handleRetrievalFinished);
    Q_EMIT searchingChanged();
}

void OnlineTicketImporter::handleRetrievalFinished()
{
    assert(m_currentJob);
    m_currentJob->deleteLater();
    m_errorMessage = m_currentJob->errorMessage();
    const auto result = m_currentJob->result();
    m_currentJob = nullptr;
    Q_EMIT searchingChanged();

    if (result.isEmpty() || !m_errorMessage.isEmpty()) {
        Q_EMIT searchFailed();
    } else {
        assert(m_resMgr);
        m_resMgr->importReservations(result);
        Q_EMIT searchSucceeded();
    }
}

QNetworkAccessManager* OnlineTicketImporter::nam()
{
    // TODO centralize that somewhere in the app, we don't really need our own QNAM here
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        m_nam->setStrictTransportSecurityEnabled(true);
        m_nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/hsts/"));
    }
    return m_nam;
}
