/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRANSFERHELPERS_H
#define TRANSFERHELPERS_H

class QDateTime;
class QVariant;

/** Helper methods for TransferManager. */
namespace TransferHelper
{

    /** Return the transfer anchor time before/after the reservation @p res. */
    QDateTime anchorTimeBefore(const QVariant &res);
    QDateTime anchorTimeAfter(const QVariant &res);
}

#endif // TRANSFERHELPERS_H
