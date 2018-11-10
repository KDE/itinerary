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

#ifndef COUNTRYMODEL_H
#define COUNTRYMODEL_H

#include <QAbstractListModel>
namespace KItinerary {
namespace KnowledgeDb {
    struct Country;
}
}

/** Country model for selecting the home country. */
class CountryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CountryModel(QObject *parent = nullptr);
    ~CountryModel();

    Q_INVOKABLE int isoCodeToIndex(const QString &isoCode) const;
    Q_INVOKABLE QString isoCodeFromIndex(int index) const;

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QVector<const KItinerary::KnowledgeDb::Country *> m_countries;
};

#endif // COUNTRYMODEL_H
