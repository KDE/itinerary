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
    std::vector<Departure> departures;
};
}

void DepartureReplyPrivate::finalizeResult()
{
    if (departures.empty()) {
        return;
    }
    error = Reply::NoError;
    errorMsg.clear();

    std::sort(departures.begin(), departures.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.scheduledTime() < rhs.scheduledTime();
    });
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

std::vector<Departure> DepartureReply::departures() const
{
    Q_D(const DepartureReply);
    return d->departures; // TODO this copies
}

void DepartureReply::addResult(std::vector<Departure> &&res)
{
    Q_D(DepartureReply);
    if (d->departures.empty()) {
        d->departures = std::move(res);
    } else {
        d->departures.insert(d->departures.end(), res.begin(), res.end());
    }

    d->pendingOps--;
    d->emitFinishedIfDone(this);
}
