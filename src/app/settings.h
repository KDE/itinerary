/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

/** Application settings accessible by QML. */
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool weatherForecastEnabled READ weatherForecastEnabled WRITE setWeatherForecastEnabled NOTIFY weatherForecastEnabledChanged)
    Q_PROPERTY(QString homeCountryIsoCode READ homeCountryIsoCode WRITE setHomeCountryIsoCode NOTIFY homeCountryIsoCodeChanged)
    Q_PROPERTY(bool queryLiveData READ queryLiveData WRITE setQueryLiveData NOTIFY queryLiveDataChanged)
    Q_PROPERTY(bool preloadMapData READ preloadMapData WRITE setPreloadMapData NOTIFY preloadMapDataChanged)

    Q_PROPERTY(bool performCurrencyConversion READ performCurrencyConversion WRITE setPerformCurrencyConversion NOTIFY performCurrencyConversionChanged)

    Q_PROPERTY(bool autoAddTransfers READ autoAddTransfers WRITE setAutoAddTransfers NOTIFY autoAddTransfersChanged)
    Q_PROPERTY(bool autoFillTransfers READ autoFillTransfers WRITE setAutoFillTransfers NOTIFY autoFillTransfersChanged)

    Q_PROPERTY(bool showNotificationOnLockScreen READ showNotificationOnLockScreen WRITE setShowNotificationOnLockScreen NOTIFY showNotificationOnLockScreenChanged)

    Q_PROPERTY(bool osmContributorMode MEMBER m_osmContributorMode WRITE setOsmContributorMode NOTIFY osmContributorModeChanged)
    Q_PROPERTY(bool developmentMode READ developmentMode WRITE setDevelopmentMode NOTIFY developmentModeChanged)

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    Q_INVOKABLE QVariant read(const QString &key, const QVariant &defaultValue) const;
    Q_INVOKABLE void write(const QString &key, const QVariant &value);

    bool weatherForecastEnabled() const;
    void setWeatherForecastEnabled(bool enabled);

    QString homeCountryIsoCode() const;
    void setHomeCountryIsoCode(const QString &isoCode);

    bool queryLiveData() const;
    void setQueryLiveData(bool queryLiveData);

    bool preloadMapData() const;
    void setPreloadMapData(bool preload);

    bool performCurrencyConversion() const;
    void setPerformCurrencyConversion(bool enable);

    bool autoAddTransfers() const;
    void setAutoAddTransfers(bool autoAdd);
    bool autoFillTransfers() const;
    void setAutoFillTransfers(bool autoFill);

    bool showNotificationOnLockScreen() const;
    void setShowNotificationOnLockScreen(bool enabled);

    void setOsmContributorMode(bool enabled);

    bool developmentMode() const;
    void setDevelopmentMode(bool enabled);

Q_SIGNALS:
    void weatherForecastEnabledChanged(bool enabled);
    void homeCountryIsoCodeChanged(const QString &isoCode);
    void queryLiveDataChanged(bool enabled);
    void preloadMapDataChanged(bool preload);
    void performCurrencyConversionChanged(bool enabled);
    void autoAddTransfersChanged(bool autoAdd);
    void autoFillTransfersChanged(bool autoFill);
    void showNotificationOnLockScreenChanged(bool enabled);
    void developmentModeChanged(bool enabled);
    void osmContributorModeChanged(bool enabled);

private:
    QString m_homeCountry;
    bool m_weatherEnabled = false;
    bool m_queryLiveData = false;
    bool m_preloadMapData = false;
    bool m_currencyConversion = false;
    bool m_autoAddTransfers = true;
    bool m_autoFillTransfers = false;
    bool m_showNotificationOnLockScreen = false;
    bool m_osmContributorMode = false;
    bool m_developmentMode = false;
};

#endif // SETTINGS_H
