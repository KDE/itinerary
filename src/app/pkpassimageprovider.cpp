/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
