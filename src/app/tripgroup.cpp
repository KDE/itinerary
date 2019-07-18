/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tripgroup.h"
#include "logging.h"
#include "reservationmanager.h"
#include "tripgroupmanager.h"

#include <KItinerary/SortUtil>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

TripGroup::TripGroup() = default;

TripGroup::TripGroup(TripGroupManager *mgr)
    : m_mgr(mgr)
{
}

TripGroup::~TripGroup() = default;

QString TripGroup::name() const
{
    return m_name;
}

void TripGroup::setName(const QString &name)
{
    m_name = name;
}

QVector<QString> TripGroup::elements() const
{
    return m_elements;
}

void TripGroup::setElements(const QVector<QString> &elems)
{
    m_elements = elems;
}

bool TripGroup::load(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open trip group file:" << path << f.errorString();
        return false;
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    const auto obj = doc.object();
    m_name = obj.value(QLatin1String("name")).toString();
    const auto elems = obj.value(QLatin1String("elements")).toArray();
    m_elements.clear();
    m_elements.reserve(elems.size());
    for (const auto &v : elems) {
        m_elements.push_back(v.toString());
    }

    return elems.size() >= 2;
}

void TripGroup::store(const QString &path) const
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to open trip group file:" << path << f.errorString();
        return;
    }

    QJsonObject obj;
    obj.insert(QLatin1String("name"), m_name);
    QJsonArray elems;
    std::copy(m_elements.begin(), m_elements.end(), std::back_inserter(elems));
    obj.insert(QLatin1String("elements"), elems);
    f.write(QJsonDocument(obj).toJson());
}

QDateTime TripGroup::beginDateTime() const
{
    const auto res = m_mgr->m_resMgr->reservation(m_elements.at(0));
    return KItinerary::SortUtil::startDateTime(res);
}

QDateTime TripGroup::endDateTime() const
{
    const auto res = m_mgr->m_resMgr->reservation(m_elements.constLast());
    return KItinerary::SortUtil::endDateTime(res);
}
