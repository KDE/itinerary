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

public:
    explicit Clipboard(QObject *parent = nullptr);

    Q_INVOKABLE void saveText(const QString &message);

private:
    QClipboard *m_clipboard;
};
