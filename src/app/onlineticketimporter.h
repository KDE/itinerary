/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ONLINETICKETIMPORTER_H
#define ONLINETICKETIMPORTER_H

#include "importcontroller.h"

#include <QObject>

class OnlineTicketRetrievalJob;

class QNetworkAccessManager;

/** QML interface for online ticket imports. */
class OnlineTicketImporter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY searchingChanged)
    Q_PROPERTY(ImportController* importController MEMBER m_importController NOTIFY importControllerChanged)

public:
    explicit OnlineTicketImporter(QObject *parent = nullptr);
    ~OnlineTicketImporter();

    bool searching() const;
    QString errorMessage() const;

    Q_INVOKABLE void search(const QString &sourceId, const QVariantMap &arguments);

    static void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory);

Q_SIGNALS:
    void searchingChanged();
    void searchSucceeded();
    void searchFailed();
    void importControllerChanged();

private:
    void handleRetrievalFinished();

    static std::function<QNetworkAccessManager*()> s_namFactory;

    ImportController *m_importController = nullptr;
    OnlineTicketRetrievalJob *m_currentJob = nullptr;
    QString m_errorMessage;
};

#endif // ONLINETICKETIMPORTER_H
