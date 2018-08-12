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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
