/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QNetworkAccessManager>

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

protected:
    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData = nullptr) override;
};
