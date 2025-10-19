// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "abstractaccount.h"

AbstractAccount::AbstractAccount(QObject *parent)
    : QObject(parent)
{}


bool AbstractAccount::handleCallback(const QUrl &url)
{
    Q_UNUSED(url);
    return false;
}

void AbstractAccount::init(const QString &identifier, const std::function<QNetworkAccessManager *()> &namFactory)
{
    m_namFactory = namFactory;
    m_identifier = identifier;
}

QString AbstractAccount::identifier() const
{
    return m_identifier;
}
