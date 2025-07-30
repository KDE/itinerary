// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <Quotient/connection.h>
#include <Quotient/keyverificationsession.h>

#include <qqmlregistration.h>

// TODO this should be upstream in libquotient
struct KeyVerificationSessionForeign
{
    Q_GADGET
    QML_UNCREATABLE("")
    QML_NAMED_ELEMENT(KeyVerificationSession)
    QML_FOREIGN(Quotient::KeyVerificationSession)
};

struct ConnectionForeign
{
    Q_GADGET
    QML_UNCREATABLE("")
    QML_NAMED_ELEMENT(Connection)
    QML_FOREIGN(Quotient::Connection)
};
