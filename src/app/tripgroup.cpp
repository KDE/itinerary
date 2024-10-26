/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroup.h"

#include "jsonio.h"
#include "logging.h"
#include "transfer.h"

#include <KItinerary/SortUtil>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;

TripGroup::TripGroup() = default;
TripGroup::~TripGroup() = default;

QString TripGroup::name() const
{
    return m_name;
}

void TripGroup::setName(const QString &name)
{
    m_name = name;
}

bool TripGroup::hasAutomaticName() const
{
    return m_automaticName;
}

void TripGroup::setNameIsAutomatic(bool automatic)
{
    m_automaticName = automatic;
}

bool TripGroup::isAutomaticallyGrouped() const
{
    return m_automaticallyGrouped;
}

void TripGroup::setIsAutomaticallyGrouped(bool automatic)
{
    m_automaticallyGrouped = automatic;
}

QList<QString> TripGroup::elements() const
{
    return m_elements;
}

void TripGroup::setElements(const QList<QString> &elems)
{
    m_elements = elems;
}

QJsonObject TripGroup::toJson(const TripGroup &group)
{
    QJsonObject obj;
    obj.insert("name"_L1, group.m_name);
    if (!group.m_automaticName) {
        obj.insert("automaticName"_L1, group.m_automaticName);
    }
    if (!group.m_automaticallyGrouped) {
        obj.insert("automaticallyGrouped"_L1, group.m_automaticallyGrouped);
    }
    QJsonArray elems;
    std::copy(group.m_elements.begin(), group.m_elements.end(), std::back_inserter(elems));
    obj.insert("elements"_L1, elems);
    obj.insert("beginDateTime"_L1, group.beginDateTime().toString(Qt::ISODate));
    obj.insert("endDateTime"_L1, group.endDateTime().toString(Qt::ISODate));
    obj.insert("matrixRoomId"_L1, group.m_matrixRoomId);
    return obj;
}

TripGroup TripGroup::fromJson(const QJsonObject &obj)
{
    TripGroup tg;
    tg.m_name = obj.value("name"_L1).toString();
    tg.m_automaticName = obj.value("automaticName"_L1).toBool(true);
    tg.m_automaticallyGrouped = obj.value("automaticallyGrouped"_L1).toBool(true);
    const auto elems = obj.value("elements"_L1).toArray();
    tg.m_elements.clear();
    tg.m_elements.reserve(elems.size());
    for (const auto &v : elems) {
        tg.m_elements.push_back(v.toString());
    }

    tg.m_beginDateTime = QDateTime::fromString(obj.value("beginDateTime"_L1).toString(), Qt::ISODate);
    tg.m_endDateTime = QDateTime::fromString(obj.value("endDateTime"_L1).toString(), Qt::ISODate);
    tg.m_matrixRoomId = obj.value("matrixRoomId"_L1).toString();
    return tg;
}

bool TripGroup::load(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open trip group file:" << path << f.errorString();
        return false;
    }

    *this = TripGroup::fromJson(JsonIO::read(f.readAll()).toObject());
    return !m_name.isEmpty();
}

void TripGroup::store(const QString &path) const
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to open trip group file:" << path << f.errorString();
        return;
    }

    f.write(JsonIO::write(TripGroup::toJson(*this)));
}

QString TripGroup::slugName() const
{
    QString s;
    s.reserve(m_name.size());
    for (const auto c : m_name) {
        if (c.isLetter() || c.isDigit()) {
            s.push_back(c.toCaseFolded());
        } else if (!s.isEmpty() && s.back() != '-'_L1) {
            s.push_back('-'_L1);
        }
    }

    if (s.endsWith('-'_L1)) {
        s.chop(1);
    }

    return s;
}

QDateTime TripGroup::beginDateTime() const
{
    return m_beginDateTime;
}

void TripGroup::setBeginDateTime(const QDateTime &beginDt)
{
    m_beginDateTime = beginDt;
}

QDateTime TripGroup::endDateTime() const
{
    return m_endDateTime;
}

void TripGroup::setEndDateTime(const QDateTime &endDt)
{
    m_endDateTime = endDt;
}

QString TripGroup::matrixRoomId() const
{
    return m_matrixRoomId;
}

void TripGroup::setMatrixRoomId(const QString &roomId)
{
    m_matrixRoomId = roomId;
}

#include "moc_tripgroup.cpp"
