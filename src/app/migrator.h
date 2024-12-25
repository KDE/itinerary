/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MIGRATOR_H

/** Configuration and data format migration. */
class Migrator
{
public:
    /** Perform all necessary migrations. */
    static void run();

private:
    static void dropTripGroupExpandCollapseState();
};

#endif
