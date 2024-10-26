/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PKPASSIMAGEPROVIDER_H
#define PKPASSIMAGEPROVIDER_H

#include <QQuickImageProvider>

#include <functional>
#include <vector>

namespace KPkPass
{
class Pass;
}

/** QML image provider for PkPass assets. */
class PkPassImageProvider : public QQuickImageProvider
{
public:
    explicit PkPassImageProvider();
    ~PkPassImageProvider();

    void registerPassProvider(std::function<KPkPass::Pass *(const QString &, const QString &)> &&passProvider);

    [[nodiscard]] QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    Q_DISABLE_COPY(PkPassImageProvider)

    std::vector<std::function<KPkPass::Pass *(const QString &, const QString &)>> m_passProviders;
};

#endif // PKPASSIMAGEPROVIDER_H
