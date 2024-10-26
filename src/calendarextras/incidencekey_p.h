/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_INCIDENCEKEY_P_H
#define KCALENDARCORE_INCIDENCEKEY_P_H

#include <QDateTime>
#include <QString>

/** A hash key for Incidences containing uid and recurrence id (if present). */
struct IncidenceKey {
    QString uid;
    QDateTime recurrenceId;

    inline bool operator==(const IncidenceKey &other) const
    {
        return uid == other.uid && (recurrenceId.isValid() == other.recurrenceId.isValid()) && (!recurrenceId.isValid() || recurrenceId == other.recurrenceId);
    }
};

namespace std
{
template<>
class hash<IncidenceKey>
{
public:
    std::size_t operator()(const IncidenceKey &key) const
    {
        return std::hash<QString>{}(key.uid) ^ std::hash<qint64>{}(key.recurrenceId.toSecsSinceEpoch());
    }
};
}

#endif
