/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COUNTRYSUBDIVISIONMODEL_H
#define COUNTRYSUBDIVISIONMODEL_H

#include <KCountry>
#include <KCountrySubdivision>

#include <QAbstractListModel>

/** Country subdivision model, for a given country. */
class CountrySubdivisionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(KCountry country READ country WRITE setCountry NOTIFY countryChanged)

public:
    enum {
        CodeRole = Qt::UserRole,
        SubdivisionRole,
    };

    explicit CountrySubdivisionModel(QObject *parent = nullptr);
    ~CountrySubdivisionModel();

    KCountry country() const;
    void setCountry(const KCountry &country);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int rowForNameOrCode(const QString &input) const;

Q_SIGNALS:
    void countryChanged();

private:
    KCountry m_country;
    QList<KCountrySubdivision> m_subdivs;
};

#endif // COUNTRYSUBDIVISIONMODEL_H
