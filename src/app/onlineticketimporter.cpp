/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "onlineticketimporter.h"
#include "onlineticketretrievaljob.h"
#include "logging.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QStandardPaths>

#include <cassert>

std::function<QNetworkAccessManager*()> OnlineTicketImporter::s_namFactory;

OnlineTicketImporter::OnlineTicketImporter(QObject *parent)
    : QObject(parent)
{
}

OnlineTicketImporter::~OnlineTicketImporter() = default;

bool OnlineTicketImporter::searching() const
{
    return m_currentJob;
}

void OnlineTicketImporter::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory)
{
    s_namFactory = namFactory;
}

QString OnlineTicketImporter::errorMessage() const
{
    return m_errorMessage;
}

void OnlineTicketImporter::search(const QString &sourceId, const QVariantMap &arguments)
{
    qCDebug(Log) << sourceId << arguments;
    delete m_currentJob;

    m_currentJob = new OnlineTicketRetrievalJob(sourceId, arguments, s_namFactory(), this);
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
        assert(m_importController);
        m_importController->importData(QJsonDocument(result).toJson(QJsonDocument::Compact));
        Q_EMIT searchSucceeded();
    }
}

#include "moc_onlineticketimporter.cpp"
