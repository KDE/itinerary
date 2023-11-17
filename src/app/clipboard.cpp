// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "clipboard.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QRegularExpression>
#include <QUrl>

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
    , m_clipboard(QGuiApplication::clipboard())
{
}

void Clipboard::saveText(const QString &message)
{
    auto text = message;
    const static QRegularExpression regex(QStringLiteral("<[^>]*>"));
    auto mineData = new QMimeData; // ownership is transferred to clipboard
    mineData->setHtml(message);
    mineData->setText(text.replace(regex, QString()));
    m_clipboard->setMimeData(mineData);
}

#include "moc_clipboard.cpp"
