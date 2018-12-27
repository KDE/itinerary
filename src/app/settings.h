/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    Q_PROPERTY(bool allowInsecureServices READ allowInsecureServices WRITE setAllowInsecureServices NOTIFY allowInsecureServicesChanged)
public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    bool weatherForecastEnabled() const;
    void setWeatherForecastEnabled(bool enabled);

    QString homeCountryIsoCode() const;
    void setHomeCountryIsoCode(const QString &isoCode);

    bool queryLiveData() const;
    void setQueryLiveData(bool queryLiveData);
    bool allowInsecureServices() const;
    void setAllowInsecureServices(bool allowInsecure);

signals:
    void weatherForecastEnabledChanged(bool enabled);
    void homeCountryIsoCodeChanged(const QString &isoCode);
    void queryLiveDataChanged(bool enabled);
    void allowInsecureServicesChanged(bool allowed);

private:
    QString m_homeCountry;
    bool m_weatherEnabled = false;
    bool m_queryLiveData = false;
    bool m_allowInsecureServices = false;
};

#endif // SETTINGS_H
