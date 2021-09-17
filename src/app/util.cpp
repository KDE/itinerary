/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "util.h"

#include <KItinerary/JsonLdDocument>

#include <KTextToHTML>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QTimeZone>

using namespace KItinerary;

Util::Util(QObject* parent)
    : QObject(parent)
{
}

Util::~Util() = default;

QDateTime Util::dateTimeStripTimezone(const QVariant& obj, const QString& propertyName) const
{
    auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (!dt.isValid()) {
        return {};
    }

    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

QVariant Util::setDateTimePreserveTimezone(const QVariant &obj, const QString& propertyName, QDateTime value) const
{
    QVariant o(obj);
    const auto oldDt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (oldDt.isValid()) {
        value.setTimeZone(oldDt.timeZone());
    }
    JsonLdDocument::writeProperty(o, propertyName.toUtf8().constData(), value);
    return o;
}

bool Util::isRichText(const QString &text) const
{
    auto idx = text.indexOf(QLatin1Char('<'));
    if (idx >= 0 && idx < text.size() - 2) {
        return text[idx + 1].isLetter() || text[idx + 1] == QLatin1Char('/');
    }
    return false;
}

QString Util::textToHtml(const QString& text) const
{
    if (isRichText(text)) {
        return text;
    }
    return KTextToHTML::convertToHtml(text, KTextToHTML::ConvertPhoneNumbers | KTextToHTML::PreserveSpaces);
}

void Util::sortModel(QObject *model, int column, Qt::SortOrder sortOrder) const
{
    if (auto qaim = qobject_cast<QAbstractItemModel*>(model)) {
        qaim->sort(column, sortOrder);
    }
}

#include "moc_util.cpp"
