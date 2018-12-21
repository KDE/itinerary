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

#include "locationreply.h"
#include "reply_p.h"
#include "locationrequest.h"
#include "logging.h"

#include <KPublicTransport/Location>

using namespace KPublicTransport;

namespace KPublicTransport {
class LocationReplyPrivate: public ReplyPrivate {
public:
    void finalizeResult() override;

    LocationRequest request;
    std::vector<Location> locations;
};
}

void LocationReplyPrivate::finalizeResult()
{
    // merge all duplicates, as there is no natural order for name searches this is done in O(n²) for now
    for (auto it = locations.begin(); it != locations.end(); ++it) {
        for (auto mergeIt = it + 1; mergeIt != locations.end();) {
            if (Location::isSame(*it, *mergeIt)) {
                *it = Location::merge(*it, *mergeIt);
                mergeIt = locations.erase(mergeIt);
            } else {
                ++mergeIt;
            }
        }
    }

    // when searching by geo coordinate, sort by distance
    if (request.hasCoordinate()) {
        std::sort(locations.begin(), locations.end(), [this](const auto &lhs, const auto &rhs) {
            return Location::distance(request.latitude(), request.longitude(), lhs.latitude(), lhs.longitude())
                 < Location::distance(request.latitude(), request.longitude(), rhs.latitude(), rhs.longitude());
        });
    } else {
        // for name based search, sort by Levenshtein distance
        // TODO
    }
}

LocationReply::LocationReply(const LocationRequest &req)
    : Reply(new LocationReplyPrivate)
{
    Q_D(LocationReply);
    d->request = req;
}

LocationReply::~LocationReply() = default;

LocationRequest LocationReply::request() const
{
    Q_D(const LocationReply);
    return d->request;
}

const std::vector<Location>& LocationReply::result() const
{
    Q_D(const LocationReply);
    return d->locations;
}

std::vector<Location>&& LocationReply::takeResult()
{
    Q_D(LocationReply);
    return std::move(d->locations);
}

void LocationReply::addResult(std::vector<Location> &&res)
{
    Q_D(LocationReply);
    if (d->locations.empty()) {
        d->locations = std::move(res);
    } else {
        d->locations.insert(d->locations.end(), res.begin(), res.end());
    }

    d->pendingOps--;
    d->emitFinishedIfDone(this);
}
