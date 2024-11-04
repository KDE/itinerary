// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "costaccumulator.h"

#include "reservationmanager.h"

#include <KItinerary/PriceUtil>
#include <KItinerary/Reservation>

#include <KUnitConversion/Value>

using namespace KItinerary;

CostAccumulator::CostAccumulator(const ReservationManager *resMgr)
    : m_resMgr(resMgr)
{
}

CostAccumulator::~CostAccumulator() = default;

void CostAccumulator::setTargetCurrency(const QString &currency)
{
    m_targetCurrency = currency;
}

Price CostAccumulator::totalCost() const
{
    if (m_targetCurrency.isEmpty()) {
        return m_totalCosts.size() == 1 ? Price{ m_totalCosts.begin().value(), m_totalCosts.begin().key() } : Price{};
    }

    Price converted;
    converted.currency = m_targetCurrency;
    for (auto it = m_totalCosts.begin(); it != m_totalCosts.end(); ++it) {
        if (it.key() == m_targetCurrency) {
            converted.value = std::isnan(converted.value) ? it.value() : (it.value() + converted.value);
            continue;
        }
        const auto v = KUnitConversion::Value(it.value(), it.key()).convertTo(m_targetCurrency);
        if (!v.isValid()) { // cannot convert one currency
            return m_totalCosts.size() == 1 ? Price{ m_totalCosts.begin().value(), m_totalCosts.begin().key() } : Price{};
        }
        converted.value = std::isnan(converted.value) ? v.number() : (v.number() + converted.value);
    }
    return converted;
}

static QString reservationNumber(const QVariant &res)
{
    if (JsonLd::canConvert<Reservation>(res)) {
        return JsonLd::convert<Reservation>(res).reservationNumber();
    }
    return {};
}

static Price costForReservation(const QVariant &res)
{
    if (!PriceUtil::hasPrice(res)) {
        return {};
    }

    return { PriceUtil::price(res), PriceUtil::currency(res) };
}

void CostAccumulator::addBatch(const QString &batchId)
{
    const auto resForBatch = m_resMgr->reservationsForBatch(batchId);

    // check if all reservations in the batch have the same price and no reservation number
    // if so, we assume the price is for all of them
    {
        const auto res = m_resMgr->reservation(batchId);
        const auto price = costForReservation(res);
        const auto resNum = reservationNumber(res);

        if (resForBatch.size() != 1 && !price.isNull() && resNum.isEmpty()) {
            const auto allEqual = std::all_of(resForBatch.begin(), resForBatch.end(), [this, price, resNum](const auto &id) {
                const auto res = m_resMgr->reservation(id);
                return costForReservation(res) == price && reservationNumber(res).isEmpty();
            });
            if (allEqual) {
                m_totalCosts[price.currency] += price.value;
                return;
            }
        }
    }

    for (const auto &resId :resForBatch) {
        const auto res = m_resMgr->reservation(resId);
        const auto price = costForReservation(res);
        if (std::isnan(price.value) || price.currency.isEmpty()) {
            continue;
        }

        const auto resNum = reservationNumber(res);
        if (resNum.isEmpty()) {
            m_totalCosts[price.currency] += price.value;
            continue;
        }

        auto &resNumEntry = m_resNumCosts[resNum];
        if (resNumEntry == price) { // we assume that this is the total price for the entire booking then
            continue;
        }
        if (resNumEntry.isNull()) {
            resNumEntry = price;
        } else {
            resNumEntry.value = -1.0;
        }

        m_totalCosts[price.currency] += price.value;
    }
}

#include "moc_costaccumulator.cpp"
