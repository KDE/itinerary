/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroup.h"

#include "jsonio.h"
#include "logging.h"
#include "transfer.h"

#include <KItinerary/SortUtil>

#include <QDateTime>
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

QList<QString> TripGroup::elements() const { return m_elements; }

void TripGroup::setElements(const QList<QString> &elems) { m_elements = elems; }

QJsonObject TripGroup::toJson(const TripGroup &group)
{
    QJsonObject obj;
    obj.insert("name"_L1, group.m_name);
    QJsonArray elems;
    std::copy(group.m_elements.begin(), group.m_elements.end(), std::back_inserter(elems));
    obj.insert("elements"_L1, elems);
    obj.insert("beginDateTime"_L1, group.beginDateTime().toString(Qt::ISODate));
    obj.insert("endDateTime"_L1, group.endDateTime().toString(Qt::ISODate));
    return obj;
}

TripGroup TripGroup::fromJson(const QJsonObject &obj)
{
    TripGroup tg;
    tg.m_name = obj.value("name"_L1).toString();
    const auto elems = obj.value("elements"_L1).toArray();
    tg.m_elements.clear();
    tg.m_elements.reserve(elems.size());
    for (const auto &v : elems) {
        tg.m_elements.push_back(v.toString());
    }

    tg.m_beginDateTime = QDateTime::fromString(obj.value("beginDateTime"_L1).toString(), Qt::ISODate);
    tg.m_endDateTime = QDateTime::fromString(obj.value("endDateTime"_L1).toString(), Qt::ISODate);

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
    return m_elements.size() >= 2;
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

#include "moc_tripgroup.cpp"
