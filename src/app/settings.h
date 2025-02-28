/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <KFormat>

#include <QObject>
#include <QUrl>

/** Application settings accessible by QML. */
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool weatherForecastEnabled READ weatherForecastEnabled WRITE setWeatherForecastEnabled NOTIFY weatherForecastEnabledChanged)
    Q_PROPERTY(QString homeCountryIsoCode READ homeCountryIsoCode WRITE setHomeCountryIsoCode NOTIFY homeCountryIsoCodeChanged)
    Q_PROPERTY(bool forceMetricUnits READ forceMetricUnits WRITE setForceMetricUnits NOTIFY forceMetricUnitsChanged)
    Q_PROPERTY(bool queryLiveData READ queryLiveData WRITE setQueryLiveData NOTIFY queryLiveDataChanged)
    Q_PROPERTY(bool preloadMapData READ preloadMapData WRITE setPreloadMapData NOTIFY preloadMapDataChanged)

    Q_PROPERTY(bool performCurrencyConversion READ performCurrencyConversion WRITE setPerformCurrencyConversion NOTIFY performCurrencyConversionChanged)
    Q_PROPERTY(bool wikimediaOnlineContentEnabled MEMBER m_wikimediaOnlineContentEnabled WRITE setWikimediaOnlineContentEnabled NOTIFY
                   wikimediaOnlineContentEnabledChanged)

    Q_PROPERTY(bool autoAddTransfers READ autoAddTransfers WRITE setAutoAddTransfers NOTIFY autoAddTransfersChanged)
    Q_PROPERTY(bool autoFillTransfers READ autoFillTransfers WRITE setAutoFillTransfers NOTIFY autoFillTransfersChanged)

    Q_PROPERTY(
        bool showNotificationOnLockScreen READ showNotificationOnLockScreen WRITE setShowNotificationOnLockScreen NOTIFY showNotificationOnLockScreenChanged)

    Q_PROPERTY(bool osmContributorMode MEMBER m_osmContributorMode WRITE setOsmContributorMode NOTIFY osmContributorModeChanged)
    Q_PROPERTY(bool developmentMode READ developmentMode WRITE setDevelopmentMode NOTIFY developmentModeChanged)

    Q_PROPERTY(bool useFahrenheit READ useFahrenheit NOTIFY useFahrenheitChanged)
    Q_PROPERTY(KFormat::DistanceFormatOptions distanceFormat READ distanceFormat NOTIFY forceMetricUnitsChanged)

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    Q_INVOKABLE [[nodiscard]] static QVariant read(const QString &key, const QVariant &defaultValue);
    Q_INVOKABLE static void write(const QString &key, const QVariant &value);

    /** Load/save file dialog directory. */
    Q_INVOKABLE [[nodiscard]] static QUrl readFileDialogFolder(const QString &key, const QUrl &defaultUrl);
    Q_INVOKABLE static void writeFileDialogFolder(const QString &key, const QUrl &url);

    [[nodiscard]] bool weatherForecastEnabled() const;
    void setWeatherForecastEnabled(bool enabled);

    [[nodiscard]] QString homeCountryIsoCode() const;
    void setHomeCountryIsoCode(const QString &isoCode);

    [[nodiscard]] bool forceMetricUnits() const;
    void setForceMetricUnits(bool force);

    [[nodiscard]] bool queryLiveData() const;
    void setQueryLiveData(bool queryLiveData);

    [[nodiscard]] bool preloadMapData() const;
    void setPreloadMapData(bool preload);

    [[nodiscard]] bool performCurrencyConversion() const;
    void setPerformCurrencyConversion(bool enable);

    void setWikimediaOnlineContentEnabled(bool enable);

    [[nodiscard]] bool autoAddTransfers() const;
    void setAutoAddTransfers(bool autoAdd);
    [[nodiscard]] bool autoFillTransfers() const;
    void setAutoFillTransfers(bool autoFill);

    [[nodiscard]] bool showNotificationOnLockScreen() const;
    void setShowNotificationOnLockScreen(bool enabled);

    void setOsmContributorMode(bool enabled);

    [[nodiscard]] bool developmentMode() const;
    void setDevelopmentMode(bool enabled);

    /** Reload settings from disk and emit change notifications.
     *  For use during restoring backups.
     */
    void reloadSettings();

    [[nodiscard]] bool useFahrenheit() const;
    [[nodiscard]] KFormat::DistanceFormatOptions distanceFormat() const;

Q_SIGNALS:
    void weatherForecastEnabledChanged(bool enabled);
    void homeCountryIsoCodeChanged(const QString &isoCode);
    void forceMetricUnitsChanged(bool force);
    void queryLiveDataChanged(bool enabled);
    void preloadMapDataChanged(bool preload);
    void performCurrencyConversionChanged(bool enabled);
    void wikimediaOnlineContentEnabledChanged(bool enabled);
    void autoAddTransfersChanged(bool autoAdd);
    void autoFillTransfersChanged(bool autoFill);
    void showNotificationOnLockScreenChanged(bool enabled);
    void developmentModeChanged(bool enabled);
    void osmContributorModeChanged(bool enabled);
    void useFahrenheitChanged();

private:
    void load();

    QString m_homeCountry;
    bool m_forceMetricUnits = false;
    bool m_weatherEnabled = false;
    bool m_queryLiveData = false;
    bool m_preloadMapData = false;
    bool m_currencyConversion = false;
    bool m_wikimediaOnlineContentEnabled = false;
    bool m_autoAddTransfers = true;
    bool m_autoFillTransfers = false;
    bool m_showNotificationOnLockScreen = false;
    bool m_osmContributorMode = false;
    bool m_developmentMode = false;
};

#endif // SETTINGS_H
