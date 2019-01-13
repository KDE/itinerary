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

#ifndef KPUBLICTRANSPORT_JOURNEYREPLY_H
#define KPUBLICTRANSPORT_JOURNEYREPLY_H

#include "reply.h"

#include <vector>

namespace KPublicTransport {

class Journey;
class JourneyReplyPrivate;
class JourneyRequest;

/** Journey query response. */
class JourneyReply : public Reply
{
    Q_OBJECT
public:
    ~JourneyReply();

    /** The request this is the reply for. */
    JourneyRequest request() const;

    /** Returns the retrieved journeys. */
    const std::vector<Journey>& result() const;
    /** Returns the retrieved journeys for moving elsewhere. */
    std::vector<Journey>&& takeResult();

private:
    friend class Manager;
    explicit JourneyReply(const JourneyRequest &req);

    friend class AbstractBackend;
    void addResult(std::vector<Journey> &&res);

    Q_DECLARE_PRIVATE(JourneyReply)
};

}

#endif // KPUBLICTRANSPORT_JOURNEYREPLY_H
