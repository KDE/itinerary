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

#ifndef KPUBLICTRANSPORT_LOCATIONREPLY_H
#define KPUBLICTRANSPORT_LOCATIONREPLY_H

#include "reply.h"

#include <vector>

namespace KPublicTransport {

class AbstractBackend;
class Location;
class LocationRequest;
class LocationReplyPrivate;

/** Location query reply. */
class LocationReply : public Reply
{
    Q_OBJECT
public:
    ~LocationReply();

    /** The request this is the reply for. */
    LocationRequest request() const;

    /** Returns the found locations. */
    const std::vector<Location>& result() const;
    /** Returns the found locations for moving elsewhere. */
    std::vector<Location>&& takeResult();

private:
    friend class Manager;
    explicit LocationReply(const LocationRequest &req);

    friend class AbstractBackend;
    void addResult(std::vector<Location> &&res);

    Q_DECLARE_PRIVATE(LocationReply)
};

}

#endif // KPUBLICTRANSPORT_LOCATIONREPLY_H
