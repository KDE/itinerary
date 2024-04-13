// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

class QClipboard;

/**
 * Clipboard proxy
 */
class Clipboard : public QObject
{
    Q_OBJECT
    /** Clipboard currently holds a text content. */
    Q_PROPERTY(bool hasText READ hasText NOTIFY contentChanged)
    /** Clipboard currently holds URLs. */
    Q_PROPERTY(bool hasUrls READ hasUrls NOTIFY contentChanged)
    /** Clipboard currently holds binary data. */
    Q_PROPERTY(bool hasBinaryData READ hasBinaryData NOTIFY contentChanged)

    /** Textual clipboard content. */
    Q_PROPERTY(QString text READ text NOTIFY contentChanged)
    /** Binary data clipboard content. */
    Q_PROPERTY(QByteArray binaryData READ binaryData NOTIFY contentChanged)

public:
    explicit Clipboard(QObject *parent = nullptr);

    [[nodiscard]] static bool hasText();
    [[nodiscard]] static bool hasUrls();
    [[nodiscard]] static bool hasBinaryData();

    [[nodiscard]] static QString text();
    [[nodiscard]] static QByteArray binaryData();

    Q_INVOKABLE static void saveText(const QString &message);

Q_SIGNALS:
    /** Clipboard content changed. */
    void contentChanged();
};
