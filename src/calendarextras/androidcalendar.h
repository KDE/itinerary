/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_ANDROIDCALENDAR_H
#define KCALENDARCORE_ANDROIDCALENDAR_H

#include <KCalendarCore/Calendar>

#include "android/calendar.h"
#include "incidencekey_p.h"

#include <jni.h>

#include <unordered_map>

/** Access to an Android system calendar. */
class AndroidCalendar : public KCalendarCore::Calendar
{
public:
    explicit AndroidCalendar(const QTimeZone &tz, jlong id);
    ~AndroidCalendar();

    // KCalendarCore::Calendar interface
    void close() override;
    bool deleteIncidenceInstances(const KCalendarCore::Incidence::Ptr &incidence) override;

    bool addEvent(const KCalendarCore::Event::Ptr &event) override;
    bool deleteEvent(const KCalendarCore::Event::Ptr &event) override;
    bool deleteEventInstances(const KCalendarCore::Event::Ptr &event) override;
    KCalendarCore::Event::List rawEvents(KCalendarCore::EventSortField sortField = KCalendarCore::EventSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
#if KCALENDARCORE_BUILD_DEPRECATED_SINCE(5, 95)
    KCalendarCore::Event::List rawEventsForDate(const QDateTime &dt) const override;
#endif
    KCalendarCore::Event::List rawEvents(const QDate &start, const QDate &end, const QTimeZone &timeZone = {}, bool inclusive = false) const override;
    KCalendarCore::Event::List rawEventsForDate(const QDate &date,  const QTimeZone &timeZone = {}, KCalendarCore::EventSortField sortField = KCalendarCore::EventSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
    KCalendarCore::Event::Ptr event(const QString &uid, const QDateTime &recurrenceId = {}) const override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KCalendarCore::Event::Ptr deletedEvent(const QString &uid, const QDateTime &recurrenceId = {}) const override;
    KCalendarCore::Event::List deletedEvents(KCalendarCore::EventSortField sortField = KCalendarCore::EventSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
#endif
    KCalendarCore::Event::List eventInstances(const KCalendarCore::Incidence::Ptr &event, KCalendarCore::EventSortField sortField = KCalendarCore::EventSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;

    bool addTodo(const KCalendarCore::Todo::Ptr &todo) override;
    bool deleteTodo(const KCalendarCore::Todo::Ptr &todo) override;
    bool deleteTodoInstances(const KCalendarCore::Todo::Ptr &todo) override;
    KCalendarCore::Todo::List rawTodos(KCalendarCore::TodoSortField sortField = KCalendarCore::TodoSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
    KCalendarCore::Todo::List rawTodosForDate(const QDate &date) const override;
    KCalendarCore::Todo::List rawTodos(const QDate &start, const QDate &end, const QTimeZone &timeZone = {}, bool inclusive = false) const override;
    KCalendarCore::Todo::Ptr todo(const QString &uid, const QDateTime &recurrenceId = {}) const override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KCalendarCore::Todo::Ptr deletedTodo(const QString &uid, const QDateTime &recurrenceId = {}) const override;
    KCalendarCore::Todo::List deletedTodos(KCalendarCore::TodoSortField sortField = KCalendarCore::TodoSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
#endif
    KCalendarCore::Todo::List todoInstances(const KCalendarCore::Incidence::Ptr &todo, KCalendarCore::TodoSortField sortField = KCalendarCore::TodoSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;

    bool addJournal(const KCalendarCore::Journal::Ptr &journal) override;
    bool deleteJournal(const KCalendarCore::Journal::Ptr &journal) override;
    bool deleteJournalInstances(const KCalendarCore::Journal::Ptr &journal) override;
    KCalendarCore::Journal::List rawJournals(KCalendarCore::JournalSortField sortField = KCalendarCore::JournalSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
    KCalendarCore::Journal::List rawJournalsForDate(const QDate &date) const override;
    KCalendarCore::Journal::Ptr journal(const QString &uid, const QDateTime &recurrenceId = {}) const override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KCalendarCore::Journal::Ptr deletedJournal(const QString &uid, const QDateTime &recurrenceId = {}) const override;
    KCalendarCore::Journal::List deletedJournals(KCalendarCore::JournalSortField sortField = KCalendarCore::JournalSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;
#endif
    KCalendarCore::Journal::List journalInstances(const KCalendarCore::Incidence::Ptr &journal, KCalendarCore::JournalSortField sortField = KCalendarCore::JournalSortUnsorted, KCalendarCore::SortDirection sortDirection = KCalendarCore::SortDirectionAscending) const override;

    KCalendarCore::Alarm::List alarms(const QDateTime &from, const QDateTime &to, bool excludeBlockedAlarms = false) const override;

    // KCalendarCore::IncidenceObserver interface
    void incidenceUpdate(const QString &uid, const QDateTime &recurrenceId) override;
    void incidenceUpdated(const QString &uid, const QDateTime &recurrenceId) override;

private:
    void registerEvents(const KCalendarCore::Event::List &events) const;
    void registerEvent(const KCalendarCore::Event::Ptr &event) const;

    JniCalendar m_calendar;
    mutable std::unordered_map<IncidenceKey, KCalendarCore::Event::Ptr> m_incidences;
};

#endif // KCALENDARCORE_ANDROIDCALENDAR_H
