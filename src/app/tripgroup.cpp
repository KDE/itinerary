/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroup.h"
#include "logging.h"
#include "reservationmanager.h"
#include "tripgroupmanager.h"
#include "transfer.h"
#include "transfermanager.h"

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
    if (m_elements.empty()) {
        return {};
    }
    const auto res = m_mgr->m_resMgr->reservation(m_elements.at(0));
    const auto dt = KItinerary::SortUtil::startDateTime(res);

    const auto transfer = m_mgr->m_transferMgr ? m_mgr->m_transferMgr->transfer(m_elements.at(0), Transfer::Before) : Transfer();
    if (transfer.state() == Transfer::Selected && transfer.journey().scheduledDepartureTime().isValid()) {
        return std::min(dt, transfer.journey().scheduledDepartureTime());
    }
    return dt;
}

QDateTime TripGroup::endDateTime() const
{
    if (m_elements.empty()) {
        return {};
    }
    const auto res = m_mgr->m_resMgr->reservation(m_elements.constLast());
    const auto dt = KItinerary::SortUtil::endDateTime(res);

    const auto transfer = m_mgr->m_transferMgr ? m_mgr->m_transferMgr->transfer(m_elements.constLast(), Transfer::After) : Transfer();
    if (transfer.state() == Transfer::Selected && transfer.journey().scheduledArrivalTime().isValid()) {
        return std::max(dt, transfer.journey().scheduledArrivalTime());
    }
    return dt;
}
