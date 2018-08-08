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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
