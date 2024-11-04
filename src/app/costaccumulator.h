// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef COSTACCUMULATOR_H
#define COSTACCUMULATOR_H

#include <QHash>
#include <QString>
#include <qobjectdefs.h>

class ReservationManager;

class Price {
    Q_GADGET
    Q_PROPERTY(double value MEMBER value)
    Q_PROPERTY(QString currency MEMBER currency)

public:
    double value = NAN;
    QString currency;

    [[nodiscard]] constexpr inline bool isNull() const
    {
        return std::isnan(value);
    }
    [[nodiscard]] inline bool operator==(const Price &other) const
    {
        return value == other.value && currency == other.currency;
    }
};

/** Determine cost of an entire trip group. */
class CostAccumulator
{
public:
    explicit CostAccumulator(const ReservationManager *resMgr);
    ~CostAccumulator();

    void setTargetCurrency(const QString &currency);

    [[nodiscard]] Price totalCost() const;

    void addBatch(const QString &batchId);

private:
    const ReservationManager *m_resMgr = nullptr;
    QString m_targetCurrency;
    QHash<QString, Price> m_resNumCosts;
    QHash<QString, double> m_totalCosts;
};

#endif
