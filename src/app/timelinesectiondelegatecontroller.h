/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMELINESECTIONDELEGATECONTROLLER_H
#define TIMELINESECTIONDELEGATECONTROLLER_H

#include "config-itinerary.h"

#if HAVE_KHOLIDAYS
#include <KHolidays/Holiday>
#endif

#include <QDate>
#include <QObject>

class TimelineModel;

/** Logic and data access for timeline section delegates.
 *  TODO this is still missing change notification for a few cases:
 *  - preceding location changes can change some or all subsequent day sections
 *  - isToday changes at midnight
 */
class TimelineSectionDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString date READ dateString WRITE setDateString NOTIFY dateChanged)
    Q_PROPERTY(TimelineModel *timelineModel READ timelineModel WRITE setTimelineModel NOTIFY timelineModelChanged)

    Q_PROPERTY(QString title READ title NOTIFY dateChanged)
    Q_PROPERTY(bool isToday READ isToday NOTIFY dateChanged)
    Q_PROPERTY(QString subTitle READ subTitle NOTIFY dateChanged)
    Q_PROPERTY(bool isHoliday READ isHoliday NOTIFY dateChanged)

public:
    explicit TimelineSectionDelegateController(QObject *parent = nullptr);
    ~TimelineSectionDelegateController();

    // limitation of what ListView can handle as a section value
    QString dateString() const;
    void setDateString(const QString &dtStr);

    TimelineModel *timelineModel() const;
    void setTimelineModel(TimelineModel *model);

    QString title() const;
    bool isToday() const;
    QString subTitle() const;
    bool isHoliday() const;

Q_SIGNALS:
    void dateChanged();
    void timelineModelChanged();

private:
    void recheckHoliday();

    TimelineModel *m_model = nullptr;
    QDate m_date;
#if HAVE_KHOLIDAYS
    KHolidays::Holiday::List m_holidays;
#endif
};

#endif // TIMELINESECTIONDELEGATECONTROLLER_H
