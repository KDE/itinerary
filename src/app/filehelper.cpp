/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filehelper.h"

#include <QString>
#include <QUrl>

#include <cstring>

bool FileHelper::isLocalFile(const QUrl &url)
{
    return url.isLocalFile() || url.scheme() == QLatin1String("content");
}

QString FileHelper::toLocalFile(const QUrl &url)
{
    return url.isLocalFile() ? url.toLocalFile() : url.toString(QUrl::FullyEncoded);
}

bool FileHelper::hasZipHeader(const QByteArray &data)
{
    return data.size() >= 4 && std::strncmp(data.constData(), "PK\x03\x04", 4) == 0;
}
