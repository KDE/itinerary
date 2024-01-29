/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEATHERFORECASTMODEL_H
#define WEATHERFORECASTMODEL_H

#include "weatherforecast.h"

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
    ~WeatherForecastModel() override;

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
