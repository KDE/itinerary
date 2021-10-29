/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "util.h"
#include "logging.h"

#include <KItinerary/JsonLdDocument>

#include <KTextToHTML>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QFile>
#include <QTimeZone>
#include <QXmlStreamReader>

#include <cmath>

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

float Util::svgAspectRatio(const QString &svgFilePath) const
{
    if (svgFilePath.isEmpty()) {
        return 1.0f;
    }

    QFile file(svgFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(Log) << file.errorString() << svgFilePath;
        return 1.0f;
    }

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        if (reader.readNext() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() != QLatin1String("svg")) {
            qCDebug(Log) << svgFilePath << "not an svg file?";
            return 1.0f;
        }
        const auto viewBox = reader.attributes().value(QLatin1String("viewBox"));
        const auto parts = viewBox.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() != 4) {
            qCDebug(Log) << "invalid SVG viewBox:" << viewBox;
            return 1.0f;
        }

        bool widthValid, heightValid;
        const auto width = parts.at(2).toDouble(&widthValid);
        const auto height = parts.at(3).toDouble(&heightValid);
        if (!widthValid || !heightValid || height == 0.0f || width == 0.0f) {
            qCDebug(Log) << "invalid SVG viewBox:" << viewBox;
            return 1.0f;
        }

        return width / height;
    }

    return 1.0f;
}

#include "moc_util.cpp"
