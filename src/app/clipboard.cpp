// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "clipboard.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QRegularExpression>
#include <QUrl>

using namespace Qt::Literals::StringLiterals;

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
{
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, &Clipboard::contentChanged);
}

bool Clipboard::hasText()
{
    return QGuiApplication::clipboard()->mimeData()->hasText();
}

bool Clipboard::hasUrls()
{
    return QGuiApplication::clipboard()->mimeData()->hasUrls();
}

bool Clipboard::hasBinaryData()
{
    return QGuiApplication::clipboard()->mimeData()->hasFormat("application/octet-stream"_L1);
}

QString Clipboard::text()
{
    return QGuiApplication::clipboard()->mimeData()->text();
}

QByteArray Clipboard::binaryData()
{
    return QGuiApplication::clipboard()->mimeData()->data("application/octet-stream"_L1);
}

void Clipboard::saveText(const QString &message)
{
    auto text = message;
    const static QRegularExpression regex(u"<[^>]*>"_s);
    auto mineData = new QMimeData; // ownership is transferred to clipboard
    mineData->setHtml(message);
    mineData->setText(text.replace(regex, QString()));
    QGuiApplication::clipboard()->setMimeData(mineData);
}

#include "moc_clipboard.cpp"
