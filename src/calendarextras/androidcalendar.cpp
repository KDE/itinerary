/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidcalendar.h"
#include "androidicalconverter.h"

#include "android/eventdata.h"

#include <QCoreApplication>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#endif

using namespace KAndroidExtras;

AndroidCalendar::AndroidCalendar(const QTimeZone &tz, jlong id)
    : KCalendarCore::Calendar(tz)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    , m_calendar(Jni::fromHandle<android::content::Context>(QtAndroid::androidContext()), id)
#else
    , m_calendar(Jni::fromHandle<android::content::Context>(QJniObject(QNativeInterface::QAndroidApplication::context())), id)
#endif
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setDeletionTracking(false);
#endif
}

AndroidCalendar::~AndroidCalendar() = default;

void AndroidCalendar::close()
{
    m_incidences.clear();
}

bool AndroidCalendar::deleteIncidenceInstances(const KCalendarCore::Incidence::Ptr &incidence)
{
    switch (incidence->type()) {
        case KCalendarCore::IncidenceBase::TypeEvent:
            return deleteEventInstances(incidence.staticCast<KCalendarCore::Event>());
        case KCalendarCore::IncidenceBase::TypeTodo:
            return deleteTodo(incidence.staticCast<KCalendarCore::Todo>());
        case KCalendarCore::IncidenceBase::TypeJournal:
            return deleteJournal(incidence.staticCast<KCalendarCore::Journal>());
        case KCalendarCore::IncidenceBase::TypeUnknown:
        case KCalendarCore::IncidenceBase::TypeFreeBusy:
            return false;
    }
    return false;
}

KCalendarCore::Event::List AndroidCalendar::rawEvents(KCalendarCore::EventSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    const auto jniEvents = m_calendar.rawEvents();
    KCalendarCore::Event::List result;
    result.reserve(jniEvents.size());
    std::transform(jniEvents.begin(), jniEvents.end(), std::back_inserter(result), &AndroidIcalConverter::readEvent);
    registerEvents(result);
    return sortEvents(std::move(result), sortField, sortDirection);
}

#if KCALENDARCORE_BUILD_DEPRECATED_SINCE(5, 95)
KCalendarCore::Event::List AndroidCalendar::rawEventsForDate(const QDateTime &dt) const
{
    return rawEventsForDate(dt.date(), dt.timeZone());
}
#endif

bool AndroidCalendar::addEvent(const KCalendarCore::Event::Ptr &event)
{
    const auto data = AndroidIcalConverter::writeEvent(event);
    const auto result = m_calendar.addEvent(data);
    if (result) {
        event->resetDirtyFields();
        registerEvent(event);
    }
    return result;
}

bool AndroidCalendar::deleteEvent(const KCalendarCore::Event::Ptr &event)
{
    m_incidences.erase({event->uid(), event->recurrenceId()});
    event->unRegisterObserver(this);

    if (event->hasRecurrenceId()) {
        return m_calendar.deleteEvent(event->uid(), event->recurrenceId().toMSecsSinceEpoch());
    }
    return m_calendar.deleteEvent(event->uid());
}

bool AndroidCalendar::deleteEventInstances(const KCalendarCore::Event::Ptr &event)
{
    event->unRegisterObserver(this);
    for (auto it = m_incidences.begin(); it != m_incidences.end();) {
        if ((*it).first.uid == event->uid()) {
            (*it).second->unRegisterObserver(this);
            it = m_incidences.erase(it);
        } else {
            ++it;
        }
    }

    return m_calendar.deleteEventInstances(event->uid());
}

KCalendarCore::Event::List AndroidCalendar::rawEvents(const QDate &start, const QDate &end, const QTimeZone &timeZone, bool inclusive) const
{
    const auto startMSec = QDateTime(start, {0, 0}, timeZone.isValid() ? timeZone : QTimeZone::systemTimeZone()).toMSecsSinceEpoch();
    const auto endMSecs = QDateTime(end, {0, 0}, timeZone.isValid() ? timeZone : QTimeZone::systemTimeZone()).addDays(1).toMSecsSinceEpoch();
    const auto jniEvents = m_calendar.rawEvents(startMSec, endMSecs, inclusive);

    KCalendarCore::Event::List result;
    result.reserve(result.size());
    std::transform(jniEvents.begin(), jniEvents.end(), std::back_inserter(result), &AndroidIcalConverter::readEvent);
    registerEvents(result);
    return result;
}

KCalendarCore::Event::List AndroidCalendar::rawEventsForDate(const QDate &date, const QTimeZone &timeZone, KCalendarCore::EventSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    return Calendar::sortEvents(rawEvents(date, date, timeZone, false), sortField, sortDirection);
}

KCalendarCore::Event::Ptr AndroidCalendar::event(const QString &uid, const QDateTime &recurrenceId) const
{
    // check if we know this one already first
    const auto it = m_incidences.find({uid, recurrenceId});
    if (it != m_incidences.end()) {
        return (*it).second;
    }

    KCalendarCore::Event::Ptr event;
    if (recurrenceId.isValid()) {
        event = AndroidIcalConverter::readEvent(m_calendar.event(uid, recurrenceId.toMSecsSinceEpoch()));
    } else {
        event = AndroidIcalConverter::readEvent(m_calendar.event(uid));
    }
    registerEvent(event);
    return event;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
KCalendarCore::Event::Ptr AndroidCalendar::deletedEvent(const QString &uid, const QDateTime& recurrenceId) const
{
    // deletion tracking not supported
    Q_UNUSED(uid);
    Q_UNUSED(recurrenceId);
    return {};
}

KCalendarCore::Event::List AndroidCalendar::deletedEvents(KCalendarCore::EventSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    // deletion tracking not supported
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}
#endif

KCalendarCore::Event::List AndroidCalendar::eventInstances(const KCalendarCore::Incidence::Ptr &event, KCalendarCore::EventSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    const auto jniEvents = m_calendar.eventInstances(event->uid());

    KCalendarCore::Event::List result;
    result.reserve(result.size());
    std::transform(jniEvents.begin(), jniEvents.end(), std::back_inserter(result), &AndroidIcalConverter::readEvent);
    registerEvents(result);
    return sortEvents(std::move(result), sortField, sortDirection);
}

//BEGIN todo interface, not available in standard Android (needs OpenTasks - https://github.com/dmfs/opentasks)
bool AndroidCalendar::addTodo(const KCalendarCore::Todo::Ptr &todo)
{
    Q_UNUSED(todo);
    return false;
}

bool AndroidCalendar::deleteTodo(const KCalendarCore::Todo::Ptr &todo)
{
    Q_UNUSED(todo);
    return false;
}

bool AndroidCalendar::deleteTodoInstances(const KCalendarCore::Todo::Ptr &todo)
{
    Q_UNUSED(todo);
    return false;
}

KCalendarCore::Todo::List AndroidCalendar::rawTodos(KCalendarCore::TodoSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}

KCalendarCore::Todo::List AndroidCalendar::rawTodosForDate(const QDate &date) const
{
    Q_UNUSED(date);
    return {};
}

KCalendarCore::Todo::List AndroidCalendar::rawTodos(const QDate &start, const QDate &end, const QTimeZone &timeZone, bool inclusive) const
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(timeZone);
    Q_UNUSED(inclusive);
    return {};
}

KCalendarCore::Todo::Ptr AndroidCalendar::todo(const QString &uid, const QDateTime &recurrenceId) const
{
    Q_UNUSED(uid);
    Q_UNUSED(recurrenceId);
    return {};
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
KCalendarCore::Todo::Ptr AndroidCalendar::deletedTodo(const QString &uid, const QDateTime& recurrenceId) const
{
    // deletion tracking not supported
    Q_UNUSED(uid);
    Q_UNUSED(recurrenceId);
    return {};
}

KCalendarCore::Todo::List AndroidCalendar::deletedTodos(KCalendarCore::TodoSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    // deletion tracking not supported
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}
#endif

KCalendarCore::Todo::List AndroidCalendar::todoInstances(const KCalendarCore::Incidence::Ptr &todo, KCalendarCore::TodoSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    Q_UNUSED(todo);
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}
//END todo interface

//BEGIN journal interface, not available on Android
bool AndroidCalendar::addJournal(const KCalendarCore::Journal::Ptr &journal)
{
    Q_UNUSED(journal);
    return false;
}

bool AndroidCalendar::deleteJournal(const KCalendarCore::Journal::Ptr &journal)
{
    Q_UNUSED(journal);
    return false;
}

bool AndroidCalendar::deleteJournalInstances(const KCalendarCore::Journal::Ptr &journal)
{
    Q_UNUSED(journal);
    return false;
}

KCalendarCore::Journal::List AndroidCalendar::rawJournals(KCalendarCore::JournalSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}

KCalendarCore::Journal::List AndroidCalendar::rawJournalsForDate(const QDate &date) const
{
    Q_UNUSED(date);
    return {};
}

KCalendarCore::Journal::Ptr AndroidCalendar::journal(const QString &uid, const QDateTime &recurrenceId) const
{
    Q_UNUSED(uid);
    Q_UNUSED(recurrenceId);
    return {};
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
KCalendarCore::Journal::Ptr AndroidCalendar::deletedJournal(const QString& uid, const QDateTime& recurrenceId) const
{
    Q_UNUSED(uid);
    Q_UNUSED(recurrenceId);
    return {};
}

KCalendarCore::Journal::List AndroidCalendar::deletedJournals(KCalendarCore::JournalSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}
#endif

KCalendarCore::Journal::List AndroidCalendar::journalInstances(const KCalendarCore::Incidence::Ptr &journal, KCalendarCore::JournalSortField sortField, KCalendarCore::SortDirection sortDirection) const
{
    Q_UNUSED(journal);
    Q_UNUSED(sortField);
    Q_UNUSED(sortDirection);
    return {};
}
//END journal interface

KCalendarCore::Alarm::List AndroidCalendar::alarms(const QDateTime &from, const QDateTime &to, bool excludeBlockedAlarms) const
{
    // TODO
    return {};
}

void AndroidCalendar::incidenceUpdate(const QString &uid, const QDateTime &recurrenceId)
{
    Q_UNUSED(uid);
    Q_UNUSED(recurrenceId);
    qDebug() << "begin updates" << uid << recurrenceId;
}

void AndroidCalendar::incidenceUpdated(const QString &uid, const QDateTime &recurrenceId)
{
    const auto it = m_incidences.find({uid, recurrenceId});
    if (it == m_incidences.end()) {
        qWarning() << "got incidece update notification for an untracked incidence!?" << uid << recurrenceId;
        return;
    }

    const auto &event = (*it).second;
    JniEventData data = AndroidIcalConverter::writeEvent(event);
    const bool result = m_calendar.updateEvent(data, event->dirtyFields().contains(KCalendarCore::IncidenceBase::FieldAlarms), event->dirtyFields().contains(KCalendarCore::IncidenceBase::FieldAttendees));
    if (result) {
        event->resetDirtyFields();
    } else {
        qWarning() << "failed to update event!" << uid << recurrenceId << event->summary();
    }
}

void AndroidCalendar::registerEvents(const KCalendarCore::Event::List &events) const
{
    for (const auto &event : events) {
        registerEvent(event);
    }
}

void AndroidCalendar::registerEvent(const KCalendarCore::Event::Ptr& event) const
{
    if (!event) {
        return;
    }
    event->registerObserver(const_cast<AndroidCalendar*>(this));
    m_incidences[{event->uid(), event->recurrenceId()}] = event;
}
