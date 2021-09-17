/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UTIL_H
#define UTIL_H

#include <QObject>

class QDateTime;

/** Misc utilities. */
class Util : public QObject
{
    Q_OBJECT
public:
    explicit Util(QObject *parent = nullptr);
    ~Util();

    // workarounds for JS not preserving timezones

    /** Read a QDateTime property with the timezone stripped off. */
    Q_INVOKABLE QDateTime dateTimeStripTimezone(const QVariant &obj, const QString &propertyName) const;
    /** Set a QDateTime property preserving the timezone of the current value. */
    Q_INVOKABLE QVariant setDateTimePreserveTimezone(const QVariant &obj, const QString &propertyName, QDateTime value) const;

    /** Checks whether @p text is rich-text. */
    Q_INVOKABLE bool isRichText(const QString &text) const;

    /** Convert links, email address and phone numbers in the given text to HTML links. */
    Q_INVOKABLE QString textToHtml(const QString &text) const;

    /** Execute the non-exported sort() method on a QAbstractItemModel. */
    Q_INVOKABLE void sortModel(QObject *model, int column, Qt::SortOrder sortOrder) const;
};

#endif // UTIL_H
