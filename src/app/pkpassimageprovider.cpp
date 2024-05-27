/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassimageprovider.h"

#include <KPkPass/Pass>

#include <QDebug>
#include <QGuiApplication>
#include <QImage>

using namespace Qt::Literals::StringLiterals;

PkPassImageProvider::PkPassImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

PkPassImageProvider::~PkPassImageProvider() = default;

void PkPassImageProvider::registerPassProvider(std::function<KPkPass::Pass*(const QString&, const QString &)> &&passProvider)
{
    m_passProviders.push_back(std::move(passProvider));
}

QImage PkPassImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)

    auto pos = id.lastIndexOf('/'_L1);
    if (pos <= 0) {
        return {};
    }
    const auto assetName = id.mid(pos + 1);
    const auto idBase = QStringView(id).left(pos);

    pos = idBase.lastIndexOf('/'_L1);
    if (pos <= 0) {
        return {};
    }

    const auto passTypeIdentifier = idBase.left(pos).toString();
    const auto serialNum = QString::fromUtf8(QByteArray::fromBase64(idBase.mid(pos + 1).toUtf8(), QByteArray::Base64UrlEncoding));

    for (const auto &passProvider : m_passProviders) {
        const auto pass = passProvider(passTypeIdentifier, serialNum);
        if (!pass) {
            continue;
        }

        auto img = pass->image(assetName, qGuiApp->devicePixelRatio()); // TODO pass this via id from Item.window.devicePixelRatio
        if (img.isNull()) {
            return {};
        }
        *size = img.size() / img.devicePixelRatio();
        return img;
    }

    qDebug() << "could not find pass:" <<passTypeIdentifier << serialNum << id;
    return {};
}
