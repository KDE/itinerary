/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filehelper.h"

#include <QString>
#include <QUrl>

#ifdef Q_OS_ANDROID
#include <kandroidextras/contentresolver.h>
#endif

#include <cstring>

using namespace Qt::Literals::StringLiterals;

bool FileHelper::isLocalFile(const QUrl &url)
{
    return url.isLocalFile() || url.scheme() == QLatin1String("content");
}

QString FileHelper::toLocalFile(const QUrl &url)
{
    return url.isLocalFile() ? url.toLocalFile() : url.toString(QUrl::FullyEncoded);
}

QString FileHelper::fileName(const QUrl &url)
{
#if defined(Q_OS_ANDROID)
    if (url.scheme() == "content"_L1) {
        return KAndroidExtras::ContentResolver::fileName(url)
    }
#endif
    return url.fileName();
}

bool FileHelper::hasZipHeader(const QByteArray &data)
{
    return data.size() >= 4 && std::strncmp(data.constData(), "PK\x03\x04", 4) == 0;
}
