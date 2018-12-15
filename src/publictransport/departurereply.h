/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KPUBLICTRANSPORT_DEPARTUREREPLY_H
#define KPUBLICTRANSPORT_DEPARTUREREPLY_H

#include "reply.h"

#include <vector>

namespace KPublicTransport {

class AbstractBackend;
class Departure;
class DepartureRequest;
class DepartureReplyPrivate;

/** Departure query reply. */
class DepartureReply : public Reply
{
    Q_OBJECT
public:
    ~DepartureReply();

    /** The request this is the reply for. */
    DepartureRequest request() const;

    /** Returns the found departure information. */
    std::vector<Departure> departures() const;

private:
    friend class Manager;
    explicit DepartureReply(const DepartureRequest &req);

    friend class AbstractBackend;
    void addResult(std::vector<Departure> &&res);

    Q_DECLARE_PRIVATE(DepartureReply)
};

}

#endif // KPUBLICTRANSPORT_DEPARTUREREPLY_H
