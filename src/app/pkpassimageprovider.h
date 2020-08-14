/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PKPASSIMAGEPROVIDER_H
#define PKPASSIMAGEPROVIDER_H

#include <QQuickImageProvider>

class PkPassManager;

class PkPassImageProvider : public QQuickImageProvider
{
public:
    PkPassImageProvider(PkPassManager *mgr);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    Q_DISABLE_COPY(PkPassImageProvider)

    PkPassManager *m_mgr;
};

#endif // PKPASSIMAGEPROVIDER_H
