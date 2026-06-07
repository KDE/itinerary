/*
    SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kjnipp.h"
#include "kjniobject.h"
#include "kjnimethod.h"
#include "kjniproperty.h"

class JniEventData : public QtJniTypes::JObject<JniEventData> {};
KJNI_DECLARE_CLASS(org, kde, kcalendarcore, EventData, JniEventData)
static_assert(QtJniTypes::Traits<JniEventData>::className() == QtJniTypes::CTString("org/kde/kcalendarcore/EventData"));
static_assert(QtJniTypes::Traits<JniEventData>::signature() == QtJniTypes::CTString("Lorg/kde/kcalendarcore/EventData;"));

namespace CalendarContract {
class CalendarColumns : public QtJniTypes::JObject<CalendarColumns> {};
}
KJNI_DECLARE_NESTED_CLASS(android, provider, CalendarContract, CalendarColumns, CalendarContract::CalendarColumns)
static_assert(QtJniTypes::Traits<CalendarContract::CalendarColumns>::className() == QtJniTypes::CTString("android/provider/CalendarContract$CalendarColumns"));
