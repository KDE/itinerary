/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMELINESECTIONDELEGATECONTROLLER_H
#define TIMELINESECTIONDELEGATECONTROLLER_H

#include <KHolidays/Holiday>

#include <QDate>
#include <QObject>

class TripGroupModel;

/** Logic and data access for timeline section delegates.
 *  TODO this is still missing change notification for a few cases:
 *  - preceding location changes can change some or all subsequent day sections
 *  - isToday changes at midnight
 */
class TimelineSectionDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString date READ dateString WRITE setDateString NOTIFY dateChanged)
    Q_PROPERTY(TripGroupModel *tripGroupModel READ tripGroupModel WRITE setTripGroupModel NOTIFY tripGroupModelChanged)

    Q_PROPERTY(QString title READ title NOTIFY dateChanged)
    Q_PROPERTY(bool isToday READ isToday NOTIFY dateChanged)
    Q_PROPERTY(QString subTitle READ subTitle NOTIFY dateChanged)
    Q_PROPERTY(bool isHoliday READ isHoliday NOTIFY dateChanged)

public:
    explicit TimelineSectionDelegateController(QObject *parent = nullptr);
    ~TimelineSectionDelegateController();

    // limitation of what ListView can handle as a section value
    [[nodiscard]] QString dateString() const;
    void setDateString(const QString &dtStr);

    [[nodiscard]] TripGroupModel *tripGroupModel() const;
    void setTripGroupModel(TripGroupModel *model);

    [[nodiscard]] QString title() const;
    [[nodiscard]] bool isToday() const;
    [[nodiscard]] QString subTitle() const;
    [[nodiscard]] bool isHoliday() const;

Q_SIGNALS:
    void dateChanged();
    void tripGroupModelChanged();

private:
    void recheckHoliday();

    TripGroupModel *m_model = nullptr;
    QDate m_date;
    KHolidays::Holiday::List m_holidays;
};

#endif // TIMELINESECTIONDELEGATECONTROLLER_H
