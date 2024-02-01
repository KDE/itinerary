/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "settings.h"

#include <KCountry>

#include <QDebug>
#include <QLocale>
#include <QSettings>

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    QSettings s;
    s.beginGroup(QLatin1StringView("Settings"));
    m_weatherEnabled = s.value(QLatin1StringView("WeatherForecastEnabled"), false).toBool();

    const auto currentCountry = KCountry::fromQLocale(QLocale().country()).alpha2();
    m_homeCountry = s.value(QLatin1StringView("HomeCountry"), currentCountry).toString();

    m_queryLiveData = s.value(QLatin1StringView("QueryLiveData"), false).toBool();

    m_preloadMapData = s.value(QLatin1StringView("PreloadMapData"), false).toBool();

    m_currencyConversion = s.value(QLatin1StringView("PerformCurrencyConversion"), false).toBool();

    m_autoAddTransfers = s.value(QLatin1StringView("AutoAddTransfers"), true).toBool();
    m_autoFillTransfers = s.value(QLatin1StringView("AutoFillTransfers"), false).toBool() && m_queryLiveData && m_autoAddTransfers;

    m_showNotificationOnLockScreen = s.value(QLatin1StringView("ShowNotificationOnLockScreen"), false).toBool();

    m_osmContributorMode = s.value(QLatin1StringView("OsmContributorMode"), false).toBool();
    m_developmentMode = s.value(QLatin1StringView("DevelopmentMode"), false).toBool();
}

Settings::~Settings() = default;

QVariant Settings::read(const QString &key, const QVariant &defaultValue) const
{
    QSettings s;
    return s.value(key, defaultValue);
}

void Settings::write(const QString& key, const QVariant& value)
{
    QSettings s;
    s.setValue(key, value);
}

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
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("WeatherForecastEnabled"), enabled);

    Q_EMIT weatherForecastEnabledChanged(enabled);
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
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("HomeCountry"), isoCode);

    Q_EMIT homeCountryIsoCodeChanged(isoCode);
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
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("QueryLiveData"), queryLiveData);

    Q_EMIT queryLiveDataChanged(queryLiveData);
}

bool Settings::preloadMapData() const
{
    return m_preloadMapData;
}

void Settings::setPreloadMapData(bool preload)
{
    if (m_preloadMapData == preload) {
        return;
    }

    m_preloadMapData = preload;
    QSettings s;
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("PreloadMapData"), preload);

    Q_EMIT preloadMapDataChanged(preload);
}

bool Settings::performCurrencyConversion() const
{
    return m_currencyConversion;
}

void Settings::setPerformCurrencyConversion(bool enable)
{
    if (m_currencyConversion == enable) {
        return;
    }

    m_currencyConversion = enable;
    QSettings s;
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("PerformCurrencyConversion"), enable);

    Q_EMIT performCurrencyConversionChanged(enable);
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
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("AutoAddTransfers"), autoAdd);

    Q_EMIT autoAddTransfersChanged(autoAdd);
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
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("AutoFillTransfers"), autoFill);

    Q_EMIT autoFillTransfersChanged(autoFill);
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
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("ShowNotificationOnLockScreen"), m_showNotificationOnLockScreen);

    Q_EMIT showNotificationOnLockScreenChanged(m_showNotificationOnLockScreen);
}

void Settings::setOsmContributorMode(bool enabled)
{
    if (m_osmContributorMode == enabled) {
        return;
    }

    m_osmContributorMode = enabled;
    QSettings s;
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("OsmContributorMode"), m_osmContributorMode);

    Q_EMIT osmContributorModeChanged(m_osmContributorMode);
}

bool Settings::developmentMode() const
{
    return m_developmentMode;
}

void Settings::setDevelopmentMode(bool enabled)
{
    if (m_developmentMode == enabled) {
        return;
    }

    m_developmentMode = enabled;
    QSettings s;
    s.beginGroup(QLatin1StringView("Settings"));
    s.setValue(QLatin1StringView("DevelopmentMode"), m_developmentMode);
    Q_EMIT developmentModeChanged(m_developmentMode);
}

#include "moc_settings.cpp"
