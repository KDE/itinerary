/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mocknetworkaccessmanager.h"

#include <QNetworkReply>

class MockNetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit MockNetworkReply(const QNetworkAccessManager::Operation op, const QNetworkRequest &request, QObject *parent);

    qint64 bytesAvailable() const override;
    void abort() override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
};

MockNetworkReply::MockNetworkReply(const QNetworkAccessManager::Operation op, const QNetworkRequest &request, QObject *parent)
    : QNetworkReply(parent)
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);
    setUrl(request.url());
    setOperation(op);
    setError(NoError, QString());

    if (!request.sslConfiguration().isNull()) {
        setSslConfiguration(request.sslConfiguration());
    }

    // TODO actually provide a result
    setError(QNetworkReply::QNetworkReply::NetworkSessionFailedError, QLatin1String("providing replies not implemented yet"));
    setFinished(true);
    QMetaObject::invokeMethod(this, &QNetworkReply::finished, Qt::QueuedConnection);
}

qint64 MockNetworkReply::bytesAvailable() const
{
    return 0;
}

void MockNetworkReply::abort()
{
}

qint64 MockNetworkReply::readData([[maybe_unused]] char *data, [[maybe_unused]] qint64 maxSize)
{
    return 0;
}

QNetworkReply* MockNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData)
{
    Request r{op, originalReq, outgoingData ? outgoingData->readAll() : QByteArray()};
    requests.push_back(std::move(r));
    return new MockNetworkReply(op, originalReq, this);
}

#include "mocknetworkaccessmanager.moc"
