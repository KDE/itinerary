/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidicalconverter.h"

#include "android/eventdata.h"

#include <KAndroidExtras/CalendarContract>
#include <KAndroidExtras/JniArray>

#include <KCalendarCore/ICalFormat>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimeZone>

#include <libical/ical.h>

using namespace Qt::Literals::StringLiterals;

namespace ical {
    using property_ptr = std::unique_ptr<icalproperty, decltype(&icalproperty_free)>;
}

KCalendarCore::Event::Ptr AndroidIcalConverter::readEvent(const JniEventData& data)
{
    if (!KAndroidExtras::Jni::handle(data).isValid()) {
        return nullptr;
    }

    KCalendarCore::ICalFormat format;
    KCalendarCore::Event::Ptr ev(new KCalendarCore::Event);
    qDebug() << data.title;

    ev->setSummary(data.title);
    ev->setLocation(data.location);
    ev->setDescription(data.description);
    ev->setAllDay(data.allDay);
    ev->setDtStart(QDateTime::fromMSecsSinceEpoch(data.dtStart).toTimeZone(QTimeZone(QString(data.startTimezone).toUtf8())));
    if (ev->allDay()) {
        // date is inclusive in iCal but non-inclusive in KCalCore
        ev->setDtEnd(QDateTime::fromMSecsSinceEpoch(data.dtEnd).toTimeZone(QTimeZone(QString(data.endTimezone).toUtf8())).addDays(-1));
    } else {
        ev->setDtEnd(QDateTime::fromMSecsSinceEpoch(data.dtEnd).toTimeZone(QTimeZone(QString(data.endTimezone).toUtf8())));
    }
    ev->setUid(data.uid2445);

    const QString duration = data.duration;
    if (!duration.isEmpty()) {
        ev->setDuration(format.durationFromString(duration));
    }

    if (!QString(data.originalId).isEmpty()) {
        ev->setRecurrenceId(QDateTime::fromMSecsSinceEpoch(data.instanceId));
    }

    const jint accessLevel = data.accessLevel;
    if (accessLevel == KAndroidExtras::EventsColumns::ACCESS_PRIVATE) { // ### we probably want to cache the constants?
        ev->setSecrecy(KCalendarCore::Event::SecrecyPrivate);
    } else if (accessLevel == KAndroidExtras::EventsColumns::ACCESS_CONFIDENTIAL) {
        ev->setSecrecy(KCalendarCore::Event::SecrecyConfidential);
    }

    const jint availability = data.availability;
    if (availability == KAndroidExtras::EventsColumns::AVAILABILITY_FREE) {
        ev->setTransparency(KCalendarCore::Event::Transparent);
    }

    KCalendarCore::Person organizer;
    organizer.setEmail(data.organizer);

    // recurrence rules
    const QString rrule = data.rrule;
    if (!rrule.isEmpty()) {
        auto r = new KCalendarCore::RecurrenceRule;
        format.fromString(r, rrule);
        ev->recurrence()->addRRule(r);
    }
    if (ev->allDay()) {
        ev->recurrence()->setRDates(AndroidIcalConverter::readRDates<QDate>(data.rdate));
    } else {
        ev->recurrence()->setRDateTimes(AndroidIcalConverter::readRDates<QDateTime>(data.rdate));
    }

    const QString exrule = data.exrule;
    if (!exrule.isEmpty()) {
        auto r = new KCalendarCore::RecurrenceRule;
        format.fromString(r, exrule);
        ev->recurrence()->addExRule(r);
    }
    if (ev->allDay()) {
        ev->recurrence()->setExDates(AndroidIcalConverter::readRDates<QDate>(data.exdate));
    } else {
        ev->recurrence()->setExDateTimes(AndroidIcalConverter::readRDates<QDateTime>(data.exdate));
    }

    // attendees
    const KAndroidExtras::Jni::Array<JniAttendeeData> attendeesData = data.attendees;
    for (const JniAttendeeData &attendeeData : attendeesData) {
        ev->addAttendee(AndroidIcalConverter::readAttendee(attendeeData));
    }

    // alarms
    const KAndroidExtras::Jni::Array<JniReminderData> remindersData = data.reminders;
    for (const JniReminderData &reminderData : remindersData) {
        ev->addAlarm(AndroidIcalConverter::readAlarm(reminderData, ev.data()));
    }

    // extended properties
    const KAndroidExtras::Jni::Array<JniExtendedPropertyData> extProperties = data.extendedProperties;
    for (const JniExtendedPropertyData &extProperty : extProperties) {
        if (QString(extProperty.name) == QLatin1StringView("vnd.android.cursor.item/vnd.ical4android.unknown-property")) {
            // DAVx⁵ extended properties: the actual name/value pair is a JSON array in the property value
            const auto a = QJsonDocument::fromJson(QString(extProperty.value).toUtf8()).array();
            if (a.size() != 2) {
                qWarning() << "Invalid DAVx⁵ extended property data:" << extProperty.value;
                continue;
            }
            AndroidIcalConverter::addExtendedProperty(ev.data(), a[0].toString(), a[1].toString());
        } else {
            qInfo() << "Unhandled extended property:" << extProperty.name << extProperty.value;
        }
    }

    ev->resetDirtyFields();
    return ev;
}

JniEventData AndroidIcalConverter::writeEvent(const KCalendarCore::Event::Ptr &event)
{
    KCalendarCore::ICalFormat format;

    JniEventData data;
    data.title = event->summary();
    data.location = event->location();
    data.description = event->description();
    // see https://developer.android.com/reference/android/provider/CalendarContract.Events.html#writing-to-events
    // for how to handle allDay events
    if (event->allDay()) {
        data.dtStart = event->dtStart().date().startOfDay(QTimeZone::utc()).toMSecsSinceEpoch();
        data.startTimezone = u"UTC"_s;
        // end date is non-inclusive, unlike what ical expects here
        data.dtEnd = event->dtEnd().date().addDays(1).startOfDay(QTimeZone::utc()).toMSecsSinceEpoch();
        data.endTimezone = u"UTC"_s;
    } else {
        data.dtStart = event->dtStart().toMSecsSinceEpoch();
        data.startTimezone = QString::fromUtf8(event->dtStart().timeZone().id());
        data.dtEnd = event->dtEnd().toMSecsSinceEpoch();
        data.endTimezone = QString::fromUtf8(event->dtEnd().timeZone().id());
    }
    data.allDay = event->allDay();
    data.uid2445 = event->uid();

    if (!event->duration().isNull()) {
        data.duration = format.toString(event->duration());
    }

    if (event->recurrenceId().isValid()) {
        data.originalId = event->uid();
        data.instanceId = event->recurrenceId().toMSecsSinceEpoch();
    }

    switch (event->secrecy()) {
        case KCalendarCore::Event::SecrecyPrivate:
            data.accessLevel = KAndroidExtras::EventsColumns::ACCESS_PRIVATE;
            break;
        case KCalendarCore::Event::SecrecyConfidential:
            data.accessLevel = KAndroidExtras::EventsColumns::ACCESS_CONFIDENTIAL;
            break;
        case KCalendarCore::Event::SecrecyPublic:
            data.accessLevel = KAndroidExtras::EventsColumns::ACCESS_PUBLIC;
            break;
    }

    switch (event->transparency()) {
        case KCalendarCore::Event::Transparent:
            data.availability = KAndroidExtras::EventsColumns::AVAILABILITY_FREE;
            break;
        case KCalendarCore::Event::Opaque:
            data.availability = KAndroidExtras::EventsColumns::AVAILABILITY_BUSY;
            break;
    }

    data.organizer = event->organizer().email();

    // recurrence rules
    if (!event->recurrence()->rRules().isEmpty()) {
        data.rrule = format.toString(event->recurrence()->rRules().at(0)); // TODO what about more than one rule?
    }
    if (event->allDay() && !event->recurrence()->rDates().isEmpty()) {
        data.rdate = writeRDates(event->recurrence()->rDates());
    } else if (!event->allDay() && !event->recurrence()->rDateTimes().isEmpty()) {
        data.rdate = writeRDates(event->recurrence()->rDateTimes());
    }
    if (!event->recurrence()->exRules().isEmpty()) {
        data.exrule = format.toString(event->recurrence()->exRules().at(0)); // TODO what about more than one rule?
    }
    if (event->allDay() && !event->recurrence()->exDates().isEmpty()) {
        data.rdate = writeRDates(event->recurrence()->exDates());
    } else if (!event->allDay() && !event->recurrence()->exDateTimes().isEmpty()) {
        data.rdate = writeRDates(event->recurrence()->exDateTimes());
    }

    // attendees
    if (!event->attendees().isEmpty()) {
        const auto attendees = event->attendees();
        KAndroidExtras::Jni::Array<JniAttendeeData> attendeeData(attendees.size());
        for (int i = 0; i < attendees.size(); ++i) {
            attendeeData[i] = writeAttendee(attendees.at(i));
        }
        data.attendees = attendeeData;
    }

    // alarms
    if (!event->alarms().isEmpty()) {
        const auto alarms = event->alarms();
        KAndroidExtras::Jni::Array<JniReminderData> reminders(alarms.size());
        for (int i = 0; i < alarms.size(); ++i) {
            reminders[i] = writeAlarm(alarms.at(i));
        }
        data.reminders = reminders;
    }

    // extended properties
    const auto properties = writeExtendedProperties(event.data());
    if (!properties.empty()) {
        KAndroidExtras::Jni::Array<JniExtendedPropertyData> propData(properties.size());
        for (std::size_t i = 0; i < properties.size(); ++i) {
            propData[i] = properties[i];
        }
        data.extendedProperties = propData;
    }

    return data;
}

KCalendarCore::Alarm::Ptr AndroidIcalConverter::readAlarm(const JniReminderData &data, KCalendarCore::Incidence *parent)
{
    KCalendarCore::Alarm::Ptr alarm(new KCalendarCore::Alarm(parent));
    alarm->setStartOffset(KCalendarCore::Duration(-data.minutes * 60, KCalendarCore::Duration::Seconds));
    if (data.method == KAndroidExtras::RemindersColumns::METHOD_EMAIL || data.method == KAndroidExtras::RemindersColumns::METHOD_SMS) {
        alarm->setType(KCalendarCore::Alarm::Email);
    } else {
        alarm->setType(KCalendarCore::Alarm::Display);
    }
    return alarm;
}

JniReminderData AndroidIcalConverter::writeAlarm(const KCalendarCore::Alarm::Ptr &alarm)
{
    JniReminderData data;
    data.minutes = -alarm->startOffset().asSeconds() / 60;
    switch (alarm->type()) {
        case KCalendarCore::Alarm::Audio:
        case KCalendarCore::Alarm::Display:
        case KCalendarCore::Alarm::Procedure:
        case KCalendarCore::Alarm::Invalid:
            data.method = KAndroidExtras::RemindersColumns::METHOD_ALERT;
            break;
        case KCalendarCore::Alarm::Email:
            data.method = KAndroidExtras::RemindersColumns::METHOD_EMAIL;
            break;
    }
    return data;
}

KCalendarCore::Attendee AndroidIcalConverter::readAttendee(const JniAttendeeData& data)
{
    KCalendarCore::Attendee attendee(data.name, data.email);

    // attendee role (### doesn't map properly, what to do about the remaining values?)
    const jint relationship = data.relationship;
    if (relationship == KAndroidExtras::AttendeesColumns::RELATIONSHIP_NONE) {
        attendee.setRole(KCalendarCore::Attendee::NonParticipant);
    } else if (relationship == KAndroidExtras::AttendeesColumns::RELATIONSHIP_ORGANIZER) {
        attendee.setRole(KCalendarCore::Attendee::Chair);
    }

    // attendee status
    const jint status = data.status;
    if (status == KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_ACCEPTED) {
        attendee.setStatus(KCalendarCore::Attendee::Accepted);
    } else if (status == KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_DECLINED) {
        attendee.setStatus(KCalendarCore::Attendee::Declined);
    } else if (status == KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_INVITED) {
        attendee.setStatus(KCalendarCore::Attendee::NeedsAction);
    } else if (status == KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_NONE) {
        attendee.setStatus(KCalendarCore::Attendee::None);
    } else if (status == KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_TENTATIVE) {
        attendee.setStatus(KCalendarCore::Attendee::Tentative);
    }

    // attendee type (### doesn't perfectly map either, and partly maps to role?)
    const jint type = data.type;
    if (type == KAndroidExtras::AttendeesColumns::TYPE_REQUIRED) {
        attendee.setRole(KCalendarCore::Attendee::ReqParticipant);
    } else if (type == KAndroidExtras::AttendeesColumns::TYPE_OPTIONAL) {
        attendee.setRole(KCalendarCore::Attendee::OptParticipant);
    } else if (type == KAndroidExtras::AttendeesColumns::TYPE_NONE) {
        attendee.setRole(KCalendarCore::Attendee::NonParticipant);
    } else if (type == KAndroidExtras::AttendeesColumns::TYPE_RESOURCE) {
        attendee.setCuType(KCalendarCore::Attendee::Resource);
    }

    return attendee;
}

JniAttendeeData AndroidIcalConverter::writeAttendee(const KCalendarCore::Attendee &attendee)
{
    JniAttendeeData data;
    data.name = attendee.name();
    data.email = attendee.email();

    switch (attendee.cuType()) {
        case KCalendarCore::Attendee::Individual:
        case KCalendarCore::Attendee::Group:
        case KCalendarCore::Attendee::Unknown:
            data.relationship = KAndroidExtras::AttendeesColumns::RELATIONSHIP_ATTENDEE;
            break;
        case KCalendarCore::Attendee::Room:
        case KCalendarCore::Attendee::Resource:
            data.type = KAndroidExtras::AttendeesColumns::TYPE_RESOURCE;
            break;
    }

    switch (attendee.role()) {
        case KCalendarCore::Attendee::ReqParticipant:
            data.type = KAndroidExtras::AttendeesColumns::TYPE_REQUIRED;
            break;
        case KCalendarCore::Attendee::OptParticipant:
            data.type = KAndroidExtras::AttendeesColumns::TYPE_OPTIONAL;
            break;
        case KCalendarCore::Attendee::NonParticipant:
            data.type = KAndroidExtras::AttendeesColumns::TYPE_NONE;
            break;
        case KCalendarCore::Attendee::Chair:
            // TODO?
            break;
    }

    switch (attendee.status()) {
        case KCalendarCore::Attendee::NeedsAction:
            data.status = KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_INVITED;
            break;
        case KCalendarCore::Attendee::Accepted:
            data.status = KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_ACCEPTED;
            break;
        case KCalendarCore::Attendee::Declined:
            data.status = KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_DECLINED;
            break;
        case KCalendarCore::Attendee::Tentative:
            data.status = KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_TENTATIVE;
            break;
        case KCalendarCore::Attendee::Delegated:
        case KCalendarCore::Attendee::InProcess:
        case KCalendarCore::Attendee::Completed:
        case KCalendarCore::Attendee::None:
            data.status = KAndroidExtras::AttendeesColumns::ATTENDEE_STATUS_NONE;
            break;
    }

    return data;
}

void AndroidIcalConverter::addExtendedProperty(KCalendarCore::Incidence *incidence, const QString &name, const QString &value)
{
    const QByteArray propString = name.toUtf8() + ':' + value.toUtf8();
    ical::property_ptr p(icalproperty_new_from_string(propString.constData()), &icalproperty_free);
    if (!p) {
        qWarning() << "Failed to parse extended property:" << name << value;
        return;
    }

    // ### we can probably reuse large parts if icalformat_p.cpp with a bit of refactoring after moving to KCalendarCore
    switch (icalproperty_isa(p.get())) {
        case ICAL_CREATED_PROPERTY:
        {
            icaldatetimeperiodtype tp;
            tp.time = icalproperty_get_created(p.get());
            incidence->setCreated(QDateTime({tp.time.year, tp.time.month, tp.time.day}, {tp.time.hour, tp.time.minute, tp.time.second}, Qt::UTC));
            break;
        }
        case ICAL_GEO_PROPERTY:
        {
            icalgeotype geo = icalproperty_get_geo(p.get());
            incidence->setGeoLatitude(geo.lat);
            incidence->setGeoLongitude(geo.lon);
            break;
        }
        case ICAL_PRIORITY_PROPERTY:
            incidence->setPriority(icalproperty_get_priority(p.get()));
            break;
        case ICAL_X_PROPERTY:
            incidence->setNonKDECustomProperty(name.toUtf8(), value);
            break;
        default:
            qWarning() << "Unhandled property type:" << name << value;
    }
}

static JniExtendedPropertyData writeExtendedProperty(const QString &name, const QString &value)
{
    JniExtendedPropertyData data;
    data.name = QStringLiteral("vnd.android.cursor.item/vnd.ical4android.unknown-property");

    QJsonArray array;
    array.push_back(name);
    array.push_back(value);
    data.value = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));

    return data;
}

static JniExtendedPropertyData writeExtendedProperty(const ical::property_ptr &prop)
{
    const auto name = QString::fromUtf8(icalproperty_kind_to_string(icalproperty_isa(prop.get())));
    const auto icalValue = icalproperty_get_value(prop.get());
    const auto value = QString::fromUtf8(icalvalue_as_ical_string(icalValue));
    return writeExtendedProperty(name, value);
 }

std::vector<JniExtendedPropertyData> AndroidIcalConverter::writeExtendedProperties(const KCalendarCore::Incidence *incidence)
{
    std::vector<JniExtendedPropertyData> props;

    if (incidence->created().isValid()) {
        const auto dt = incidence->created().toUTC();
        icaltimetype t = icaltime_null_time();
        t.year = dt.date().year();
        t.month = dt.date().month();
        t.day = dt.date().day();
        t.hour = dt.time().hour();
        t.minute = dt.time().minute();
        t.second = dt.time().second();
        t.zone = nullptr; // zone is NOT set
        t = icaltime_convert_to_zone(t, icaltimezone_get_utc_timezone());
        ical::property_ptr p(icalproperty_new_created(t), &icalproperty_free);
        props.push_back(::writeExtendedProperty(p));
    }

    if (incidence->hasGeo()) {
        icalgeotype geo;
        geo.lat = incidence->geoLatitude();
        geo.lon = incidence->geoLongitude();
        ical::property_ptr p(icalproperty_new_geo(geo), &icalproperty_free);
        props.push_back(::writeExtendedProperty(p));
    }

    // TODO all other properties not included in the standard fields

    const auto customProps = incidence->customProperties();
    for (auto it = customProps.begin(); it != customProps.end(); ++it) {
        props.push_back(::writeExtendedProperty(QString::fromUtf8(it.key()), it.value()));
    }
    return props;
}

template<typename T>
QList<T> AndroidIcalConverter::readRDates(const QString &data)
{
    if (data.isEmpty()) {
        return {};
    }

    const auto pos = data.indexOf(QLatin1Char(';'));
    QTimeZone tz;
    QStringView listData;
    if (pos > 0) {
        listData = QStringView(data).mid(pos + 1);
        tz = QTimeZone(QStringView(data).left(pos).toUtf8());
    } else {
        listData = QStringView(data);
        tz = QTimeZone::utc();
    }

    const auto list = listData.split(QLatin1Char(','));
    QList<T> result;
    result.reserve(list.size());
    for (const auto &s : list) {
        T value;
        if constexpr (std::is_same_v<QDate, T>) {
            value = QDate::fromString(s.toString(), QStringLiteral("yyyyMMdd"));
        } else {
            if (s.endsWith(QLatin1Char('Z'))) {
                value = QDateTime::fromString(s.toString(), QStringLiteral("yyyyMMddThhmmssZ"));
                value.setTimeZone(QTimeZone::utc());
            } else {
                value = QDateTime::fromString(s.toString(), QStringLiteral("yyyyMMddThhmmss"));
                value.setTimeZone(tz);
            }
        }
        if (!value.isValid()) {
            qWarning() << "Failed to parse RDATE data:" << data;
            return {};
        }
        result.push_back(value);
    }

    return result;
}

QString AndroidIcalConverter::writeRDates(const QList<QDate> &rdates)
{
    QString result;
    result.reserve((rdates.size() * 9) - 1);
    for (const auto &rdate : rdates) {
        if (!result.isEmpty()) {
            result += QLatin1Char(',');
        }
        result += rdate.toString(QStringLiteral("yyyyMMdd"));
    }
    return result;
}

QString AndroidIcalConverter::writeRDates(const QList<QDateTime> &rdates)
{
    if (rdates.isEmpty()) {
        return {};
    }

    QString result;
    const auto tz = rdates.at(0).timeZone();
    const auto isUtc = tz == QTimeZone::utc();
    if (!isUtc) {
        result += QString::fromUtf8(tz.id()) + QLatin1Char(';');
    }

    result.reserve(result.size() + (rdates.size() * (isUtc ? 14 : 13)) - 1);
    for (int i = 0; i < rdates.size(); ++i) {
        if (i > 0) {
            result += QLatin1Char(',');
        }
        result += rdates.at(i).toTimeZone(tz).toString(isUtc ? QStringLiteral("yyyyMMddThhmmssZ") : QStringLiteral("yyyyMMddThhmmss"));
    }

    return result;
}
