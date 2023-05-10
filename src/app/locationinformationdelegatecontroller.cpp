/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "locationinformationdelegatecontroller.h"
#include "logging.h"

#if HAVE_KUNITCONVERSION
#include <KUnitConversion/Value>
#endif

#include <KLocalizedString>

#include <QScopedValueRollback>

LocationInformationDelegateController::LocationInformationDelegateController(QObject *parent)
    : QObject(parent)
{
}

LocationInformationDelegateController::~LocationInformationDelegateController() = default;

LocationInformation LocationInformationDelegateController::locationInformation() const
{
    return m_info;
}

void LocationInformationDelegateController::setLocationInformation(const LocationInformation &info)
{
    m_info = info;
    Q_EMIT infoChanged();
    recheckCurrencyConversion();
}

QString LocationInformationDelegateController::homeCurrencyCode() const
{
    return m_homeCurrency;
}

void LocationInformationDelegateController::setHomeCurrencyCode(const QString &currencyCode)
{
    if (m_homeCurrency == currencyCode) {
        return;
    }

    m_homeCurrency = currencyCode;
    Q_EMIT homeCurrencyCodeChanged();
    recheckCurrencyConversion();
}

bool LocationInformationDelegateController::performCurrencyConversion() const
{
    return m_performCurrencyConverion;
}

void LocationInformationDelegateController::setPerformCurrencyConversion(bool enable)
{
    if (m_performCurrencyConverion == enable) {
        return;
    }

    m_performCurrencyConverion = enable;
    Q_EMIT performCurrencyConversionChanged();
    recheckCurrencyConversion();
}

bool LocationInformationDelegateController::hasCurrencyConversion() const
{
    return m_performCurrencyConverion && m_conversionRate > 0.0f;
}

QString LocationInformationDelegateController::currencyConversionLabel() const
{
    return i18nc("currency conversion rate", "1 %1 = %2 %3", m_homeCurrency, m_conversionRate, m_info.currencyCode());
}

void LocationInformationDelegateController::recheckCurrencyConversion()
{
    // HACK work around KUnitConversion's internal currency update event loop causing us to re-enter here again
    // which would then subsequently deadlock in KUnitConversion
    static bool s_entrancyGuard = false;
    if (s_entrancyGuard) {
        qCWarning(Log) << "Imminent KUnitConversion deadlock detected, skipping currency conversion!";
        return;
    }
    QScopedValueRollback rollback(s_entrancyGuard, false);
    s_entrancyGuard = true;

    float rate = 0.0f;
#if HAVE_KUNITCONVERSION
    if (m_performCurrencyConverion && m_info.currencyDiffers() && !m_homeCurrency.isEmpty()) {
        const auto value = KUnitConversion::Value(1.0, m_homeCurrency).convertTo(m_info.currencyCode());
        if (value.isValid()) {
            rate = value.number();
        }
    }
#endif

    if (rate != m_conversionRate) {
        m_conversionRate = rate;
        Q_EMIT currencyConversionChanged();
    }
}
