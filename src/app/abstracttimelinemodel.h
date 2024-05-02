// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>

class ReservationManager;

class AbstractTimelineModel : public QAbstractListModel
{
public:
    explicit AbstractTimelineModel(QObject *parent);

    [[nodiscard]]
    virtual ReservationManager *reservationManager() const = 0;
};
