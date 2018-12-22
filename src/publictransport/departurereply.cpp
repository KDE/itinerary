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

#include "departurereply.h"
#include "reply_p.h"
#include "departurerequest.h"
#include "logging.h"

#include <KPublicTransport/Departure>

#include <QNetworkReply>

using namespace KPublicTransport;

namespace KPublicTransport {
class DepartureReplyPrivate : public ReplyPrivate {
public:
    void finalizeResult() override;

    DepartureRequest request;
    std::vector<Departure> result;
};
}

void DepartureReplyPrivate::finalizeResult()
{
    if (result.empty()) {
        return;
    }
    error = Reply::NoError;
    errorMsg.clear();

    if (request.mode() == DepartureRequest::QueryDeparture) {
        std::sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.scheduledDepartureTime() < rhs.scheduledDepartureTime();
        });
    } else {
        std::sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.scheduledArrivalTime() < rhs.scheduledArrivalTime();
        });
    }

    for (auto it = result.begin(); it != result.end(); ++it) {
        for (auto mergeIt = it + 1; mergeIt != result.end();) {
            if (request.mode() == DepartureRequest::QueryDeparture) {
                if ((*it).scheduledDepartureTime() != (*mergeIt).scheduledDepartureTime()) {
                    break;
                }
            } else {
                if ((*it).scheduledArrivalTime() != (*mergeIt).scheduledArrivalTime()) {
                    break;
                }
            }

            if (Departure::isSame(*it, *mergeIt)) {
                *it = Departure::merge(*it, *mergeIt);
                mergeIt = result.erase(mergeIt);
            } else {
                ++mergeIt;
            }
        }
    }
}

DepartureReply::DepartureReply(const DepartureRequest &req)
    : Reply(new DepartureReplyPrivate)
{
    Q_D(DepartureReply);
    d->request = req;
}

DepartureReply::~DepartureReply() = default;

DepartureRequest DepartureReply::request() const
{
    Q_D(const DepartureReply);
    return d->request;
}

const std::vector<Departure>& DepartureReply::result() const
{
    Q_D(const DepartureReply);
    return d->result;
}

std::vector<Departure>&& DepartureReply::takeResult()
{
    Q_D(DepartureReply);
    return std::move(d->result);
}

void DepartureReply::addResult(std::vector<Departure> &&res)
{
    Q_D(DepartureReply);
    if (d->result.empty()) {
        d->result = std::move(res);
    } else {
        d->result.insert(d->result.end(), res.begin(), res.end());
    }

    d->pendingOps--;
    d->emitFinishedIfDone(this);
}
