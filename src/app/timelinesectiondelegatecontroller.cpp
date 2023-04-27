/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timelinesectiondelegatecontroller.h"

#include "locationhelper.h"
#include "timelinemodel.h"

#include <KHolidays/HolidayRegion>

#include <KLocalizedString>

#include <QDebug>
#include <QLocale>

TimelineSectionDelegateController::TimelineSectionDelegateController(QObject *parent)
    : QObject(parent)
{
    connect(this, &TimelineSectionDelegateController::timelineModelChanged,
            this, &TimelineSectionDelegateController::dateChanged);
}

TimelineSectionDelegateController::~TimelineSectionDelegateController() = default;

QString TimelineSectionDelegateController::dateString() const
{
    return m_date.toString(Qt::ISODate);
}

void TimelineSectionDelegateController::setDateString(const QString &dtStr)
{
    const auto dt = QDate::fromString(dtStr, Qt::ISODate);
    if (dt == m_date) {
        return;
    }

    m_date = dt;
    recheckHoliday();
    Q_EMIT dateChanged();
}

TimelineModel* TimelineSectionDelegateController::timelineModel() const
{
    return m_model;
}

void TimelineSectionDelegateController::setTimelineModel(TimelineModel *model)
{
    if (m_model == model) {
        return;
    }
    m_model = model;
    recheckHoliday();
    Q_EMIT timelineModelChanged();
}

QString TimelineSectionDelegateController::title() const
{
    if (m_model && m_date == m_model->today()) {
        return i18n("Today");
    }
    return i18nc("weekday, date", "%1, %2", QLocale().dayName(m_date.dayOfWeek(), QLocale::LongFormat), QLocale().toString(m_date, QLocale::ShortFormat));
}

bool TimelineSectionDelegateController::isToday() const
{
    return m_model ? m_date == m_model->today() : false;
}

QString TimelineSectionDelegateController::subTitle() const
{
    if (!m_holidays.isEmpty()) {
        return m_holidays.at(0).name();
    }

    return {};
}

bool TimelineSectionDelegateController::isHoliday() const
{
    // non-workdays being first is ensured recheckHoliday
    return !m_holidays.isEmpty() && m_holidays.at(0).dayType() == KHolidays::Holiday::NonWorkday;
}

void TimelineSectionDelegateController::recheckHoliday()
{
    if (!m_model || !m_date.isValid()) {
        return;
    }

    m_holidays.clear();

    const auto regionCode = LocationHelper::regionCode(m_model->locationAtTime(QDateTime(m_date, {})));
    if (regionCode.isEmpty()) {
        return;
    }

    const auto holidayRegionCode = KHolidays::HolidayRegion::defaultRegionCode(regionCode);
    if (holidayRegionCode.isEmpty()) {
        return;
    }

    const auto holidayRegion = KHolidays::HolidayRegion(holidayRegionCode);
    if (holidayRegion.isValid()) {
        m_holidays = holidayRegion.rawHolidaysWithAstroSeasons(m_date);
        // prioritize non-workdays
        std::sort(m_holidays.begin(), m_holidays.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.dayType() == KHolidays::Holiday::NonWorkday && rhs.dayType() == KHolidays::Holiday::Workday;
        });
    }
}

#include "moc_timelinesectiondelegatecontroller.cpp"
