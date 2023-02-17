/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UTIL_H
#define UTIL_H

#include <qobjectdefs.h>

class QColor;
class QDateTime;
class QVariant;

/** Misc utilities. */
class Util
{
    Q_GADGET
public:
    // workarounds for JS not preserving timezones
    /** Read a QDateTime property with the timezone stripped off. */
    Q_INVOKABLE static QDateTime dateTimeStripTimezone(const QVariant &obj, const QString &propertyName);
    /** Set a QDateTime property preserving the timezone of the current value. */
    Q_INVOKABLE static QVariant setDateTimePreserveTimezone(const QVariant &obj, const QString &propertyName, QDateTime value);

    /** Checks whether @p text is rich-text. */
    Q_INVOKABLE static bool isRichText(const QString &text);

    /** Convert links, email address and phone numbers in the given text to HTML links. */
    Q_INVOKABLE static QString textToHtml(const QString &text);
    /** Convert a phone number into a tel: link. */
    Q_INVOKABLE static QString telephoneUrl(const QString &phoneNumber);
    /** Convert an email address into a mailto: link. */
    Q_INVOKABLE static QString emailUrl(const QString &emailAddress);

    /** Execute the non-exported sort() method on a QAbstractItemModel. */
    Q_INVOKABLE static void sortModel(QObject *model, int column, Qt::SortOrder sortOrder);

    /** Determine the aspect ratio of an SVG file.
     *  This is a dirty workaround for the problem that Kirigami.Icon has its implicit size hardcoded to 32x32...
     */
    Q_INVOKABLE static float svgAspectRatio(const QString &svgFilePath);

    /** QColor::isValid for QML. */
    Q_INVOKABLE static bool isValidColor(const QColor &color);
};

#endif // UTIL_H
