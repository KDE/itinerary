/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mocknetworkaccessmanager.h"

#include <cstring>

class MockNetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit MockNetworkReply(const QNetworkAccessManager::Operation op,
                              const QNetworkRequest &request,
                              const MockNetworkAccessManager::Reply &replyData,
                              QObject *parent);

    qint64 bytesAvailable() const override;
    void abort() override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;

private:
    QByteArray m_data;
    qint64 m_offset = 0;
};

MockNetworkReply::MockNetworkReply(const QNetworkAccessManager::Operation op,
                                   const QNetworkRequest &request,
                                   const MockNetworkAccessManager::Reply &replyData,
                                   QObject *parent)
    : QNetworkReply(parent)
    , m_data(replyData.data)
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);
    setUrl(request.url());
    setOperation(op);
    setError(NoError, QString());

    if (!request.sslConfiguration().isNull()) {
        setSslConfiguration(request.sslConfiguration());
    }

    setError(replyData.error, replyData.errorMessage);
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, replyData.statusCode);
    setFinished(true);
    QMetaObject::invokeMethod(this, &QNetworkReply::finished, Qt::QueuedConnection);
}

qint64 MockNetworkReply::bytesAvailable() const
{
    return QNetworkReply::bytesAvailable() + m_data.size() - m_offset;
}

void MockNetworkReply::abort()
{
}

qint64 MockNetworkReply::readData(char *data, qint64 maxSize)
{
    const qint64 length = std::min(qint64(m_data.length() - m_offset), maxSize);

    if (length <= 0) {
        return 0;
    }

    std::memcpy(data, m_data.constData() + m_offset, length);
    m_offset += length;
    return length;
}

QNetworkReply *MockNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData)
{
    Request r{op, originalReq, outgoingData ? outgoingData->readAll() : QByteArray()};
    requests.push_back(std::move(r));

    Reply replyData;
    if (!replies.empty()) {
        replyData = replies.front();
        replies.pop();
    }
    return new MockNetworkReply(op, originalReq, replyData, this);
}

#include "mocknetworkaccessmanager.moc"

#include "moc_mocknetworkaccessmanager.cpp"
