/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <queue>
#include <vector>

class MockNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    struct Request {
        QNetworkAccessManager::Operation op;
        QNetworkRequest request;
        QByteArray data;
    };

    std::vector<Request> requests;

    struct Reply {
        QNetworkReply::NetworkError error = QNetworkReply::NetworkSessionFailedError;
        int statusCode = 0;
        QByteArray data;
        QString errorMessage = QStringLiteral("no pending reply available");
    };

    std::queue<Reply> replies;

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData = nullptr) override;
};
