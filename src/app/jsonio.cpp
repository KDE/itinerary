/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonio.h"

#include <QCborValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <private/qjson_p.h>

static bool isCbor(const QByteArray &data)
{
    return !data.isEmpty() && data[0] != '{' && data[0] != '[';
}

QJsonValue JsonIO::read(const QByteArray &data)
{
    if (isCbor(data)) {
        return QJsonPrivate::Value::fromTrustedCbor(QCborValue::fromCbor(data));
    }

    const auto doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        return doc.array();
    }
    if (doc.isObject()) {
        return doc.object();
    }
    return {};
}

QByteArray JsonIO::write(const QJsonValue &value, OutputFormat format)
{
    switch (format) {
        case JSON:
            if (value.isArray()) {
                return QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact);
            }
            if (value.isObject()) {
                return QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact);
            }
            return {};
        case CBOR:
            return QCborValue::fromJsonValue(value).toCbor();
    }
    return {};
}

QByteArray JsonIO::convert(const QByteArray &data, OutputFormat format)
{
    if (isCbor(data)) {
        return format == CBOR ? data : write(read(data), format);
    }
    return format == JSON ? data : write(read(data), format);
}
