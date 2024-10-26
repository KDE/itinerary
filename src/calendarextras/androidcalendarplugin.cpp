/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidcalendarplugin.h"
#include "androidcalendar.h"

#include "android/calendardata.h"

#include "kandroidextras/calendarcontract.h"
#include <KAndroidExtras/JniObject>

#include <QCoreApplication>
#include <QDebug>

using namespace KCalendarCore;
using namespace KAndroidExtras;

AndroidCalendarPlugin::AndroidCalendarPlugin(QObject *parent, const QVariantList &args)
    : CalendarPlugin(parent, args)
    , m_jni(Jni::fromHandle<android::content::Context>(QJniObject(QNativeInterface::QAndroidApplication::context())))
{
}

AndroidCalendarPlugin::~AndroidCalendarPlugin() = default;

QList<KCalendarCore::Calendar::Ptr> AndroidCalendarPlugin::calendars() const
{
    if (m_calendars.isEmpty()) {
        loadCalendars();
    }

    return m_calendars;
}

void AndroidCalendarPlugin::loadCalendars() const
{
    const Jni::Array<JniCalendarData> cals = m_jni.getCalendars();
    for (const JniCalendarData &calData : cals) {
        auto *cal = new AndroidCalendar(QTimeZone(QString(calData.timezone).toUtf8()), calData.owner, calData.id);
        cal->setId(QString::number(calData.id));
        cal->setName(calData.displayName);

        const int accessLevel = calData.accessLevel;
        if (accessLevel == CalendarColumns::CAL_ACCESS_ROOT || accessLevel == CalendarColumns::CAL_ACCESS_OWNER
            || accessLevel == CalendarColumns::CAL_ACCESS_EDITOR || accessLevel == CalendarColumns::CAL_ACCESS_CONTRIBUTOR) {
            cal->setAccessMode(KCalendarCore::ReadWrite);
        } else {
            cal->setAccessMode(KCalendarCore::ReadOnly);
        }

        // TODO calendar color

        m_calendars.push_back(KCalendarCore::Calendar::Ptr(cal));
    }
}

#include "moc_androidcalendarplugin.cpp"
