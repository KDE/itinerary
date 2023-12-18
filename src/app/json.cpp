/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "json.h"
#include "logging.h"

#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>
#include <QTimeZone>
#include <QUrl>
#include <QVariant>

#include <cmath>

static QJsonValue variantToJson(const QVariant &v)
{
    switch (v.userType()) {
        case QMetaType::QString:
        {
            const auto s = v.toString();
            return s.isNull() ? QJsonValue() : v.toString();
        }
        case QMetaType::Double:
        case QMetaType::Float:
        {
            auto d = v.toDouble();
            if (std::isnan(d)) {
                return QJsonValue::Null;
            }
            return d;
        }
        case QMetaType::Int:
            return v.toInt();
        case QMetaType::QDateTime:
        {
            const auto dt = v.toDateTime();
            if (!dt.isValid()) {
                return {};
            }
            if (dt.timeSpec() == Qt::TimeZone) {
                QJsonObject dtObj;
                dtObj.insert(QStringLiteral("value"), dt.toString(Qt::ISODate));
                dtObj.insert(QStringLiteral("timezone"), QString::fromUtf8(dt.timeZone().id()));
                return dtObj;
            }
            return v.toDateTime().toString(Qt::ISODate);
        }
        case QMetaType::QUrl:
        {
            const auto url = v.toUrl();
            return url.isValid() ? url.toString() : QJsonValue();
        }
        case QMetaType::QColor:
        {
            const auto c = v.value<QColor>();
            return c.isValid() ? v.value<QColor>().name() : QJsonValue();;
        }
    }

    if (v.canConvert<QVariantList>()) {
        const auto l = v.toList();
        if (l.isEmpty()) {
            return {};
        }

        QJsonArray a;
        std::transform(l.begin(), l.end(), std::back_inserter(a), variantToJson);
        return a;
    }

    return {};
}

QJsonObject Json::toJson(const QMetaObject *mo, const void *elem)
{
    QJsonObject obj;

    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }

        if (prop.isFlagType()) { // flag has to come first, as prop.isEnumType() is also true for this
            const auto key = prop.readOnGadget(elem).toInt();
            const auto value = prop.enumerator().valueToKeys(key);
            obj.insert(QString::fromUtf8(prop.name()), QString::fromUtf8(value));
            continue;
        }
        if (prop.isEnumType()) { // enums defined in this QMO
            const auto key = prop.readOnGadget(elem).toInt();
            const auto value = prop.enumerator().valueToKey(key);
            obj.insert(QString::fromUtf8(prop.name()), QString::fromUtf8(value));
            continue;
        } else if (QMetaType(prop.userType()).flags() & QMetaType::IsEnumeration) { // external enums
            obj.insert(QString::fromUtf8(prop.name()), prop.readOnGadget(elem).toString());
            continue;
        }

        const auto v = variantToJson(prop.readOnGadget(elem));
        if (!v.isNull()) {
            obj.insert(QString::fromUtf8(prop.name()), v);
        }
    }

    return obj;
}

static QVariant variantFromJson(const QJsonValue &v, int mt)
{
    switch (mt) {
        case QMetaType::QString:
            return v.toString();
        case QMetaType::Double:
        case QMetaType::Float:
            return v.toDouble();
        case QMetaType::Int:
            return v.toInt();
        case QMetaType::QDateTime:
        {
            if (v.isObject()) {
                const auto dtObj = v.toObject();
                auto valueIt = dtObj.find(QLatin1String("value"));
                if (valueIt == dtObj.end()) {
                    valueIt = dtObj.find(QLatin1String("@value")); // JSON-LD compat
                }
                if (valueIt == dtObj.end()) {
                    return {};
                }
                auto dt = QDateTime::fromString(valueIt.value().toString(), Qt::ISODate);
                dt.setTimeZone(QTimeZone(dtObj.value(QLatin1String("timezone")).toString().toUtf8()));
                return dt;
            }
            return QDateTime::fromString(v.toString(), Qt::ISODate);
        }
        case QMetaType::QUrl:
            return QUrl(v.toString());
        case QMetaType::QStringList:
        {
            const auto a = v.toArray();
            QStringList l;
            l.reserve(a.size());
            for (const auto &av : a) {
                l.push_back(av.toString());
            }
            return l;
        }
        case QMetaType::QColor:
            return QColor(v.toString());
    }

    return {};
}

void Json::fromJson(const QMetaObject *mo, const QJsonObject &obj, void *elem)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const auto idx = mo->indexOfProperty(it.key().toUtf8().constData());
        if (idx < 0) {
            continue;
        }

        const auto prop = mo->property(idx);
        if (!prop.isStored()) {
            continue;
        }

        if (prop.isFlagType() && it.value().isString()) {
            const auto key = prop.enumerator().keysToValue(it.value().toString().toUtf8().constData());
            prop.writeOnGadget(elem, key);
            continue;
        }
        if (prop.isEnumType() && it.value().isString()) { // internal enums in this QMO
            const auto key = prop.enumerator().keyToValue(it.value().toString().toUtf8().constData());
            prop.writeOnGadget(elem, key);
            continue;
        }
        if ((QMetaType(prop.userType()).flags() & QMetaType::IsEnumeration) && it.value().isString()) { // external enums
            const QMetaType mt(prop.userType());
            const auto mo = mt.metaObject();
            if (!mo) {
                qCWarning(Log) << "No meta object found for enum type:" << prop.typeName();
                continue;
            }
            const auto enumIdx = mo->indexOfEnumerator(prop.typeName() + strlen(mo->className()) + 2);
            if (enumIdx < 0) {
                qCWarning(Log) << "Could not find QMetaEnum for" << prop.typeName();
                continue;
            }
            const auto me = mo->enumerator(enumIdx);
            bool success = false;
            const auto numValue = me.keyToValue(it.value().toString().toUtf8().constData(), &success);
            if (!success) {
                qCWarning(Log) << "Unknown enum value" << it.value().toString() << "for" << prop.typeName();
                continue;
            }
            auto valueData = mt.create();
            *reinterpret_cast<int*>(valueData) = numValue;
            QVariant value(prop.metaType(), valueData);
            prop.writeOnGadget(elem, value);
            continue;
        }

        const auto v = variantFromJson(it.value(), prop.userType());
        prop.writeOnGadget(elem, v);
    }
}
