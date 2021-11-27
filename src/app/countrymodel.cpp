/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "countrymodel.h"

#include <KCountry>
#include <QCollator>

CountryModel::CountryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_countries = KCountry::allCountries();
    QCollator collator;
    std::sort(m_countries.begin(), m_countries.end(), [&collator](const auto &lhs, const auto &rhs) { return collator.compare(lhs.name(), rhs.name()) < 0; });
}

CountryModel::~CountryModel() = default;

int CountryModel::isoCodeToIndex(const QString &isoCode) const
{
    const auto id = KCountry::fromAlpha2(isoCode);
    const auto it = std::find(m_countries.constBegin(), m_countries.constEnd(), id);
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
    return m_countries.at(index).alpha2();
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

    const auto country = m_countries.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return country.name();
        case Qt::EditRole:
            return country.alpha2();
    }

    return {};
}
