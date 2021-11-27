/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COUNTRYMODEL_H
#define COUNTRYMODEL_H

#include <QAbstractListModel>

class KCountry;

/** Country model for selecting the home country. */
class CountryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CountryModel(QObject *parent = nullptr);
    ~CountryModel() override;

    Q_INVOKABLE int isoCodeToIndex(const QString &isoCode) const;
    Q_INVOKABLE QString isoCodeFromIndex(int index) const;

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QList<KCountry> m_countries;
};

#endif // COUNTRYMODEL_H
