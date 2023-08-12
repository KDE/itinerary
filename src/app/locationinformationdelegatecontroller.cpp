/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "locationinformationdelegatecontroller.h"
#include "localizer.h"
#include "unitconversion.h"

#include <KLocalizedString>

#include <cmath>

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
    return i18nc("currency conversion rate", "%1 = %2", Localizer::formatCurrency(1.0, m_homeCurrency), Localizer::formatCurrency(m_conversionRate, m_info.currencyCode()));
}

void LocationInformationDelegateController::recheckCurrencyConversion()
{
    float rate = 0.0f;
    if (m_performCurrencyConverion && m_info.currencyDiffers() && !m_homeCurrency.isEmpty()) {
        const auto value = UnitConversion::convertCurrency(1.0, m_homeCurrency, m_info.currencyCode());
        if (!std::isnan(value)) {
            rate = value;
        }
    }

    if (rate != m_conversionRate) {
        m_conversionRate = rate;
        Q_EMIT currencyConversionChanged();
    }
}
