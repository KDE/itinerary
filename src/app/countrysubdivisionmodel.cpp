/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "countrysubdivisionmodel.h"

#include <QDebug>

CountrySubdivisionModel::CountrySubdivisionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

CountrySubdivisionModel::~CountrySubdivisionModel() = default;

KCountry CountrySubdivisionModel::country() const
{
    return m_country;
}

void CountrySubdivisionModel::setCountry(const KCountry &country)
{
    if (m_country == country) {
        return;
    }

    beginResetModel();
    m_country = country;
    if (m_country.isValid()) {
        m_subdivs = country.subdivisions();
        std::sort(m_subdivs.begin(), m_subdivs.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.name().localeAwareCompare(rhs.name()) < 0;
        });
    }
    endResetModel();
    Q_EMIT countryChanged();
}

int CountrySubdivisionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_subdivs.size();
}

QVariant CountrySubdivisionModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_subdivs.at(index.row()).name();
    case CodeRole:
        return m_subdivs.at(index.row()).code();
    }

    return {};
}

QHash<int, QByteArray> CountrySubdivisionModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(CodeRole, "code");
    r.insert(SubdivisionRole, "subdivision");
    return r;
}

int CountrySubdivisionModel::rowForNameOrCode(const QString &input) const
{
    for (auto i = 0; i < m_subdivs.size(); ++i) {
        const auto subdiv = m_subdivs[i];
        if (subdiv.code().compare(input, Qt::CaseInsensitive) == 0 || QStringView(subdiv.code()).mid(3).compare(input, Qt::CaseInsensitive) == 0
            || subdiv.name().compare(input, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }
    return -1;
}

#include "moc_countrysubdivisionmodel.cpp"
