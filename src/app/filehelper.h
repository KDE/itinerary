/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FILEHELPER_H
#define FILEHELPER_H

class QByteArray;
class QString;
class QUrl;

/** File related helper methods. */
namespace FileHelper
{

/** Returns whether @p url is a file that can be read by QFile directly without downloading.
 *  @note This differs from QUrl::isLocalFile in also supporting Android content: URLs.
 */
bool isLocalFile(const QUrl &url);

/** Returns a path for @p url that QFile can work with.
 *  @note This differs from QUrl::toLocalFile in also supporting Android content: URLs.
 */
QString toLocalFile(const QUrl &url);

/** Checks whether @p data starts with the magic header of a ZIP file. */
bool hasZipHeader(const QByteArray &data);

}

#endif // FILEHELPER_H
