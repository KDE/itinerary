/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassimageprovider.h"
#include "pkpassmanager.h"

#include <KPkPass/Pass>

#include <QDebug>
#include <QGuiApplication>
#include <QImage>

PkPassImageProvider::PkPassImageProvider(PkPassManager *mgr)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_mgr(mgr)
{
}

QImage PkPassImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);

    const int pos = id.lastIndexOf(QLatin1Char('/'));
    if (pos < 0)
        return {};
    const auto passId = id.left(pos);
    const auto assetName = id.mid(pos + 1);
    auto pass = m_mgr->pass(passId);
    if (!pass)
        return {};
    const auto img = pass->image(assetName, qGuiApp->devicePixelRatio()); // TODO pass this via id from Item.window.devicePixelRatio
    if (img.isNull())
        return {};
    *size = img.size() / img.devicePixelRatio();
    return img;
}
