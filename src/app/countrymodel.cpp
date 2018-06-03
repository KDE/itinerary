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

CountryModel::CountryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

CountryModel::~CountryModel() = default;

int CountryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return std::distance(KnowledgeDb::countriesBegin(), KnowledgeDb::countriesEnd());
}

QVariant CountryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto it = KnowledgeDb::countriesBegin() + index.row();
    switch (role) {
        case Qt::DisplayRole:
            return KContacts::Address::ISOtoCountry((*it).id.toString());
        case Qt::EditRole:
            return (*it).id.toString();
    }

    return {};
}
