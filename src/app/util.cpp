/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "util.h"
#include "logging.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/LocationUtil>

#include <KContacts/PhoneNumber>

#include <KTextToHTML>

#include <QAbstractItemModel>
#include <QColor>
#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QTimeZone>
#include <QUrl>

#include <cmath>

using namespace KItinerary;

QDateTime Util::dateTimeStripTimezone(const QVariant& obj, const QString& propertyName)
{
    auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (!dt.isValid()) {
        return {};
    }

    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

QVariant Util::setDateTimePreserveTimezone(const QVariant &obj, const QString& propertyName, QDateTime value)
{
    if (!value.isValid()) {
        qCDebug(Log) << "Invalid date passed for property" << propertyName;
        return obj;
    }

    QVariant o(obj);
    const auto oldDt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (oldDt.isValid()) {
        value.setTimeZone(oldDt.timeZone());
    }
    JsonLdDocument::writeProperty(o, propertyName.toUtf8().constData(), value);
    return o;
}

bool Util::isStartOfDay(const QVariant &obj, const QString &propertyName)
{
    const auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    return dt.isValid() && dt.time() == QTime(0, 0);
}

bool Util::isRichText(const QString &text)
{
    auto idx = text.indexOf(QLatin1Char('<'));
    if (idx >= 0 && idx < text.size() - 2) {
        return text[idx + 1].isLetter() || text[idx + 1] == QLatin1Char('/');
    }
    return false;
}

QString Util::textToHtml(const QString& text)
{
    if (isRichText(text)) {
        return text;
    }
    return KTextToHTML::convertToHtml(text, KTextToHTML::ConvertPhoneNumbers | KTextToHTML::PreserveSpaces);
}

QString Util::telephoneUrl(const QString &phoneNumber)
{
    return QLatin1StringView("tel:") + KContacts::PhoneNumber(phoneNumber).normalizedNumber();
}

QString Util::emailUrl(const QString &emailAddress)
{
    return QLatin1StringView("mailto:") + emailAddress;
}

void Util::sortModel(QObject *model, int column, Qt::SortOrder sortOrder)
{
    if (auto qaim = qobject_cast<QAbstractItemModel*>(model)) {
        qaim->sort(column, sortOrder);
    }
}

bool Util::isValidColor(const QColor &color)
{
    return color.isValid();
}

bool Util::isLocationChange(const QVariant &res)
{
    return LocationUtil::isLocationChange(res);
}

bool Util::isDarkImage(const QImage &img)
{
    constexpr auto SAMPLE_COUNT = 32;

    const auto xStride = std::max(1, img.width() / SAMPLE_COUNT);
    const auto yStride = std::max(1, img.height() / SAMPLE_COUNT);
    int darkCount = 0;

    for (auto x = xStride; x < img.width(); x += xStride) {
        for (auto y = yStride; y < img.height(); y += yStride) {
            if (qGray(img.pixel(x, y)) < 128) {
                ++darkCount;
            }
        }
    }

    return darkCount > (SAMPLE_COUNT * SAMPLE_COUNT / 2);
}

#include "moc_util.cpp"
