/*
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QAbstractListModel>

/// Display the number of months in a year.
class YearModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int year READ year WRITE setYear NOTIFY yearChanged)

public:
    explicit YearModel(QObject *parent = nullptr);
    ~YearModel();

    int year() const;
    void setYear(int year);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void yearChanged();

private:
    int m_year;
};
