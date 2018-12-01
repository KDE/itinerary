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

#ifndef WEATHERFORECASTMODEL_H
#define WEATHERFORECASTMODEL_H

#include <weatherforecast.h>

#include <QAbstractListModel>

class WeatherForecastManager;

/** Weather forecast details page model. */
class WeatherForecastModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* weatherForecastManager READ weatherForecastManager WRITE setWeatherForecastManager)
    Q_PROPERTY(QVariant weatherForecast READ weatherForecast WRITE setWeatherForecast)
public:
    enum Roles {
        WeatherForecastRole = Qt::UserRole,
        LocalizedTimeRole
    };

    explicit WeatherForecastModel(QObject *parent = nullptr);
    ~WeatherForecastModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QObject* weatherForecastManager() const;
    void setWeatherForecastManager(QObject *mgr);
    QVariant weatherForecast() const;
    void setWeatherForecast(const QVariant &fc);

private:
    WeatherForecastManager *m_mgr = nullptr;
    WeatherForecast m_fc;
};

#endif // WEATHERFORECASTMODEL_H
