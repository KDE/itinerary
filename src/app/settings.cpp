/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settings.h"

#include <KContacts/Address>

#include <QDebug>
#include <QLocale>
#include <QSettings>

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    m_weatherEnabled = s.value(QLatin1String("WeatherForecastEnabled"), false).toBool();

    const auto currentCountry = KContacts::Address::countryToISO(QLocale::countryToString(QLocale().country())).toUpper();
    m_homeCountry = s.value(QLatin1String("HomeCountry"), currentCountry).toString();

    m_queryLiveData = s.value(QLatin1String("QueryLiveData"), false).toBool();

    m_autoAddTransfers = s.value(QLatin1String("AutoAddTransfers"), true).toBool();
    m_autoFillTransfers = s.value(QLatin1String("AutoFillTransfers"), false).toBool() && m_queryLiveData && m_autoAddTransfers;

    m_showNotificationOnLockScreen = s.value(QLatin1String("ShowNotificationOnLockScreen"), false).toBool();
}

Settings::~Settings() = default;

bool Settings::weatherForecastEnabled() const
{
    return m_weatherEnabled;
}

void Settings::setWeatherForecastEnabled(bool enabled)
{
    if (m_weatherEnabled == enabled) {
        return;
    }

    m_weatherEnabled = enabled;
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    s.setValue(QLatin1String("WeatherForecastEnabled"), enabled);

    emit weatherForecastEnabledChanged(enabled);
}

QString Settings::homeCountryIsoCode() const
{
    return m_homeCountry;
}

void Settings::setHomeCountryIsoCode(const QString& isoCode)
{
    if (m_homeCountry == isoCode) {
        return;
    }

    m_homeCountry = isoCode;
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    s.setValue(QLatin1String("HomeCountry"), isoCode);

    emit homeCountryIsoCodeChanged(isoCode);
}

bool Settings::queryLiveData() const
{
    return m_queryLiveData;
}

void Settings::setQueryLiveData(bool queryLiveData)
{
    if (!queryLiveData) {
        setAutoFillTransfers(false);
    }

    if (m_queryLiveData == queryLiveData) {
        return;
    }

    m_queryLiveData = queryLiveData;
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    s.setValue(QLatin1String("QueryLiveData"), queryLiveData);

    emit queryLiveDataChanged(queryLiveData);
}

bool Settings::autoAddTransfers() const
{
    return m_autoAddTransfers;
}

void Settings::setAutoAddTransfers(bool autoAdd)
{
    if (!autoAdd) {
        setAutoFillTransfers(false);
    }

    if (m_autoAddTransfers == autoAdd) {
        return;
    }

    m_autoAddTransfers = autoAdd;
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    s.setValue(QLatin1String("AutoAddTransfers"), autoAdd);

    emit autoAddTransfersChanged(autoAdd);
}

bool Settings::autoFillTransfers() const
{
    return m_autoFillTransfers && m_queryLiveData && m_autoAddTransfers;
}

void Settings::setAutoFillTransfers(bool autoFill)
{
    if (m_autoFillTransfers == autoFill) {
        return;
    }

    m_autoFillTransfers = autoFill;
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    s.setValue(QLatin1String("AutoFillTransfers"), autoFill);

    emit autoFillTransfersChanged(autoFill);
}

bool Settings::showNotificationOnLockScreen() const
{
    return m_showNotificationOnLockScreen;
}

void Settings::setShowNotificationOnLockScreen(bool enabled)
{
    if (m_showNotificationOnLockScreen == enabled) {
        return;
    }

    m_showNotificationOnLockScreen = enabled;
    QSettings s;
    s.beginGroup(QLatin1String("Settings"));
    s.setValue(QLatin1String("ShowNotificationOnLockScreen"), m_showNotificationOnLockScreen);

    emit showNotificationOnLockScreenChanged(m_showNotificationOnLockScreen);
}
