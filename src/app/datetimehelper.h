/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DATETIMEHELPER_H
#define DATETIMEHELPER_H

class QDateTime;

namespace DateTimeHelper
{
/* Compare times without assuming times without a timezone are in the current time zone
 * (they might be local to the destination instead).
 */
bool isSameDateTime(const QDateTime &lhs, const QDateTime &rhs);
};

#endif
