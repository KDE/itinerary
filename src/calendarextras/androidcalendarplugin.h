/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_ANDROIDCALENDARPLUGIN_H
#define KCALENDARCORE_ANDROIDCALENDARPLUGIN_H

#include "android/calendarplugin.h"

#include <KCalendarCore/CalendarPlugin>

/** Android system calendar plugin. */
class AndroidCalendarPlugin : public KCalendarCore::CalendarPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kcalendarcore.CalendarPlugin")
public:
    explicit AndroidCalendarPlugin (QObject *parent = nullptr, const QVariantList &args = {});
    ~AndroidCalendarPlugin();

    QVector<KCalendarCore::Calendar::Ptr> calendars() const override;

private:
    void loadCalendars() const;

    mutable QVector<KCalendarCore::Calendar::Ptr> m_calendars;
    JniCalendarPlugin m_jni;
};

#endif // KCALENDARCORE_ANDROIDCALENDARPLUGIN_H
