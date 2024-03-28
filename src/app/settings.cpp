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

using namespace Qt::Literals::StringLiterals;

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    QSettings s;
    s.beginGroup("Settings"_L1);
    m_weatherEnabled = s.value("WeatherForecastEnabled"_L1, false).toBool();

    const auto currentCountry = KCountry::fromQLocale(QLocale().country()).alpha2();
    m_homeCountry = s.value("HomeCountry"_L1, currentCountry).toString();

    m_queryLiveData = s.value("QueryLiveData"_L1, false).toBool();

    m_preloadMapData = s.value("PreloadMapData"_L1, false).toBool();

    m_currencyConversion = s.value("PerformCurrencyConversion"_L1, false).toBool();

    m_autoAddTransfers = s.value("AutoAddTransfers"_L1, true).toBool();
    m_autoFillTransfers = s.value("AutoFillTransfers"_L1, false).toBool() && m_queryLiveData && m_autoAddTransfers;

    m_showNotificationOnLockScreen = s.value("ShowNotificationOnLockScreen"_L1, false).toBool();

    m_osmContributorMode = s.value("OsmContributorMode"_L1, false).toBool();
    m_developmentMode = s.value("DevelopmentMode"_L1, false).toBool();
}

Settings::~Settings() = default;

QVariant Settings::read(const QString &key, const QVariant &defaultValue)
{
    QSettings s;
    return s.value(key, defaultValue);
}

void Settings::write(const QString& key, const QVariant& value)
{
    QSettings s;
    s.setValue(key, value);
}

QUrl Settings::readFileDialogFolder(const QString &key, const QUrl &defaultUrl)
{
    QSettings s;
    s.beginGroup("FileDialog"_L1);
    return QUrl(s.value(key, defaultUrl.toString(QUrl::FullyEncoded)).toString());
}

void Settings::writeFileDialogFolder(const QString &key, const QUrl &url)
{
    QSettings s;
    s.beginGroup("FileDialog"_L1);
    s.setValue(key, url.resolved(QUrl("."_L1)).toString(QUrl::FullyEncoded)); // drop the file name
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
    s.beginGroup("Settings"_L1);
    s.setValue("WeatherForecastEnabled"_L1, enabled);

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
    s.beginGroup("Settings"_L1);
    s.setValue("HomeCountry"_L1, isoCode);

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
    s.beginGroup("Settings"_L1);
    s.setValue("QueryLiveData"_L1, queryLiveData);

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
    s.beginGroup("Settings"_L1);
    s.setValue("PreloadMapData"_L1, preload);

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
    s.beginGroup("Settings"_L1);
    s.setValue("PerformCurrencyConversion"_L1, enable);

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
    s.beginGroup("Settings"_L1);
    s.setValue("AutoAddTransfers"_L1, autoAdd);

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
    s.beginGroup("Settings"_L1);
    s.setValue("AutoFillTransfers"_L1, autoFill);

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
    s.beginGroup("Settings"_L1);
    s.setValue("ShowNotificationOnLockScreen"_L1, m_showNotificationOnLockScreen);

    Q_EMIT showNotificationOnLockScreenChanged(m_showNotificationOnLockScreen);
}

void Settings::setOsmContributorMode(bool enabled)
{
    if (m_osmContributorMode == enabled) {
        return;
    }

    m_osmContributorMode = enabled;
    QSettings s;
    s.beginGroup("Settings"_L1);
    s.setValue("OsmContributorMode"_L1, m_osmContributorMode);

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
    s.beginGroup("Settings"_L1);
    s.setValue("DevelopmentMode"_L1, m_developmentMode);
    Q_EMIT developmentModeChanged(m_developmentMode);
}

#include "moc_settings.cpp"
