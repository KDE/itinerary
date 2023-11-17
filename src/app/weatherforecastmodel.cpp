/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "weatherforecastmodel.h"

#include <weatherforecastmanager.h>

#include <QDateTime>
#include <QDebug>

WeatherForecastModel::WeatherForecastModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

WeatherForecastModel::~WeatherForecastModel() = default;

QObject* WeatherForecastModel::weatherForecastManager() const
{
    return m_mgr;
}

void WeatherForecastModel::setWeatherForecastManager(QObject* mgr)
{
    beginResetModel();
    m_mgr = qobject_cast<WeatherForecastManager*>(mgr);
    endResetModel();
}

QVariant WeatherForecastModel::weatherForecast() const
{
    return QVariant::fromValue(m_fc);
}

void WeatherForecastModel::setWeatherForecast(const QVariant& fc)
{
    beginResetModel();
    m_fc = fc.value<WeatherForecast>();
    endResetModel();
}

int WeatherForecastModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_mgr)
        return 0;
    return m_fc.range();
}

QVariant WeatherForecastModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_mgr || !m_fc.isValid())
        return {};

    switch (role) {
        case WeatherForecastRole:
        {
            const auto fc = m_mgr->forecast(m_fc.tile().latitude(), m_fc.tile().longitude(), m_fc.dateTime().addSecs(index.row() * 3600));
            return QVariant::fromValue(fc);
        }
        case LocalizedTimeRole:
            return QLocale().toString(m_fc.dateTime().addSecs(index.row() * 3600).toLocalTime().time(), QLocale::ShortFormat);
    }

    return {};
}

QHash<int, QByteArray> WeatherForecastModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(WeatherForecastRole, "weatherForecast");
    names.insert(LocalizedTimeRole, "localizedTime");
    return names;
}

#include "moc_weatherforecastmodel.cpp"
