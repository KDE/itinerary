/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settings.h"

#include <KCountry>

#include <QDebug>
#include <QLocale>
#include <QSettings>

using namespace Qt::Literals::StringLiterals;

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    m_homeCountry = KCountry::fromQLocale(QLocale().territory()).alpha2();
    load();
}

Settings::~Settings() = default;

void Settings::load()
{
    QSettings s;
    s.beginGroup("Settings"_L1);
    m_weatherEnabled = s.value("WeatherForecastEnabled"_L1, m_weatherEnabled).toBool();

    m_homeCountry = s.value("HomeCountry"_L1, m_homeCountry).toString();
    m_forceMetricUnits = s.value("ForceMetricUnits"_L1, m_forceMetricUnits).toBool();

    m_queryLiveData = s.value("QueryLiveData"_L1, m_queryLiveData).toBool();

    m_preloadMapData = s.value("PreloadMapData"_L1, m_preloadMapData).toBool();

    m_currencyConversion = s.value("PerformCurrencyConversion"_L1, m_currencyConversion).toBool();
    m_wikimediaOnlineContentEnabled = s.value("WikimediaOnlineContent"_L1, m_wikimediaOnlineContentEnabled).toBool();

    m_autoAddTransfers = s.value("AutoAddTransfers"_L1, m_autoAddTransfers).toBool();
    m_autoFillTransfers = s.value("AutoFillTransfers"_L1, m_autoFillTransfers).toBool() && m_queryLiveData && m_autoAddTransfers;

    m_showNotificationOnLockScreen = s.value("ShowNotificationOnLockScreen"_L1, m_showNotificationOnLockScreen).toBool();

    m_osmContributorMode = s.value("OsmContributorMode"_L1, m_osmContributorMode).toBool();
    m_developmentMode = s.value("DevelopmentMode"_L1, m_developmentMode).toBool();

    connect(this, &Settings::homeCountryIsoCodeChanged, this, &Settings::useFahrenheitChanged);
    connect(this, &Settings::forceMetricUnitsChanged, this, &Settings::useFahrenheitChanged);
}

void Settings::reloadSettings()
{
    load();

    Q_EMIT weatherForecastEnabledChanged(m_weatherEnabled);
    Q_EMIT homeCountryIsoCodeChanged(m_homeCountry);
    Q_EMIT forceMetricUnitsChanged(m_forceMetricUnits);
    Q_EMIT queryLiveDataChanged(m_queryLiveData);
    Q_EMIT preloadMapDataChanged(m_preloadMapData);
    Q_EMIT performCurrencyConversionChanged(m_currencyConversion);
    Q_EMIT wikimediaOnlineContentEnabledChanged(m_wikimediaOnlineContentEnabled);
    Q_EMIT autoAddTransfersChanged(m_autoAddTransfers);
    Q_EMIT autoFillTransfersChanged(m_autoFillTransfers);
    Q_EMIT showNotificationOnLockScreenChanged(m_showNotificationOnLockScreen);
    Q_EMIT developmentModeChanged(m_developmentMode);
    Q_EMIT osmContributorModeChanged(m_osmContributorMode);
}

QVariant Settings::read(const QString &key, const QVariant &defaultValue)
{
    QSettings s;
    return s.value(key, defaultValue);
}

void Settings::write(const QString &key, const QVariant &value)
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

void Settings::setHomeCountryIsoCode(const QString &isoCode)
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

bool Settings::forceMetricUnits() const
{
    return m_forceMetricUnits;
}

void Settings::setForceMetricUnits(bool force)
{
    if (m_forceMetricUnits == force) {
        return;
    }

    m_forceMetricUnits = force;
    QSettings s;
    s.beginGroup("Settings"_L1);
    s.setValue("ForceMetricUnits"_L1, force);

    Q_EMIT forceMetricUnitsChanged(force);
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

void Settings::setWikimediaOnlineContentEnabled(bool enable)
{
    if (m_wikimediaOnlineContentEnabled == enable) {
        return;
    }

    m_wikimediaOnlineContentEnabled = enable;
    QSettings s;
    s.beginGroup("Settings"_L1);
    s.setValue("WikimediaOnlineContent"_L1, m_wikimediaOnlineContentEnabled);
    Q_EMIT wikimediaOnlineContentEnabledChanged(enable);
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

bool Settings::useFahrenheit() const
{
    return m_homeCountry == "US"_L1 && !m_forceMetricUnits;
}

KFormat::DistanceFormatOptions Settings::distanceFormat() const
{
    if (QLocale().measurementSystem() != QLocale::MetricSystem && m_forceMetricUnits) {
        return KFormat::MetricDistanceUnits;
    }
    return KFormat::LocaleDistanceUnits;
}

#include "moc_settings.cpp"
