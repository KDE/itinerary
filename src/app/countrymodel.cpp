/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "countrymodel.h"

#include <KItinerary/CountryDb>

#include <KContacts/Address>

using namespace KItinerary;

static QString countryName(const KnowledgeDb::Country *country)
{
    return KContacts::Address::ISOtoCountry(country->id.toString());
}

CountryModel::CountryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_countries.resize(std::distance(KnowledgeDb::countriesBegin(), KnowledgeDb::countriesEnd()));
    std::transform(KnowledgeDb::countriesBegin(), KnowledgeDb::countriesEnd(), m_countries.begin(), [](const KnowledgeDb::Country& country) { return &country; });
    std::sort(m_countries.begin(), m_countries.end(), [](const KnowledgeDb::Country *lhs, const KnowledgeDb::Country *rhs) { return countryName(lhs) < countryName(rhs); });
}

CountryModel::~CountryModel() = default;

int CountryModel::isoCodeToIndex(const QString &isoCode) const
{
    const auto id = KnowledgeDb::CountryId{isoCode};
    const auto it = std::find_if(m_countries.constBegin(), m_countries.constEnd(), [id](const KnowledgeDb::Country *country) {
        return country->id == id;
    });
    if (it == m_countries.constEnd()) {
        return -1;
    }
    return std::distance(m_countries.constBegin(), it);
}

QString CountryModel::isoCodeFromIndex(int index) const
{
    if (index < 0 || index >= m_countries.size()) {
        return {};
    }
    return m_countries.at(index)->id.toString();
}

int CountryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_countries.count();
}

QVariant CountryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto* country = m_countries.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return countryName(country);
        case Qt::EditRole:
            return country->id.toString();
    }

    return {};
}
