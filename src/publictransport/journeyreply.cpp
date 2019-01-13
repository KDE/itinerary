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

#include "journeyreply.h"
#include "reply_p.h"
#include "journeyrequest.h"
#include "logging.h"

#include <KPublicTransport/Journey>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QTimeZone>

using namespace KPublicTransport;

namespace KPublicTransport {
class JourneyReplyPrivate : public ReplyPrivate {
public:
    void finalizeResult() override;
    void postProcessJourneys();

    JourneyRequest request;
    std::vector<Journey> journeys;
};
}

void JourneyReplyPrivate::finalizeResult()
{
    if (journeys.empty()) {
        return;
    }

    error = Reply::NoError;
    errorMsg.clear();

    postProcessJourneys();
    std::sort(journeys.begin(), journeys.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.scheduledDepartureTime() < rhs.scheduledDepartureTime();
    });
}

void JourneyReplyPrivate::postProcessJourneys()
{
    // try to fill gaps in timezone data
    for (auto &journey : journeys) {
        auto sections = journey.takeSections();
        for (auto &section : sections) {
            if (section.mode() == JourneySection::Walking) {
                if (!section.from().timeZone().isValid() && section.to().timeZone().isValid()) {
                    auto from = section.from();
                    from.setTimeZone(section.to().timeZone());
                    section.setFrom(from);
                    auto dt = section.scheduledDepartureTime();
                    dt.setTimeZone(from.timeZone());
                    section.setScheduledDepartureTime(dt);
                }
                if (section.from().timeZone().isValid() && !section.to().timeZone().isValid()) {
                    auto to = section.to();
                    to.setTimeZone(section.from().timeZone());
                    section.setTo(to);
                    auto dt = section.scheduledArrivalTime();
                    dt.setTimeZone(to.timeZone());
                    section.setScheduledArrivalTime(dt);
                }
            }
        }
        journey.setSections(std::move(sections));
    }

    // sort and merge results
    std::sort(journeys.begin(), journeys.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.scheduledDepartureTime() < rhs.scheduledDepartureTime();
    });
    for (auto it = journeys.begin(); it != journeys.end(); ++it) {
        for (auto mergeIt = it + 1; mergeIt != journeys.end();) {
            if ((*it).scheduledDepartureTime() != (*mergeIt).scheduledDepartureTime()) {
                break;
            }

            if (Journey::isSame(*it, *mergeIt)) {
                *it = Journey::merge(*it, *mergeIt);
                mergeIt = journeys.erase(mergeIt);
            } else {
                ++mergeIt;
            }
        }
    }
}

JourneyReply::JourneyReply(const JourneyRequest &req)
    : Reply(new JourneyReplyPrivate)
{
    Q_D(JourneyReply);
    d->request = req;
}

JourneyReply::~JourneyReply() = default;

JourneyRequest JourneyReply::request() const
{
    Q_D(const JourneyReply);
    return d->request;
}

const std::vector<Journey>& JourneyReply::result() const
{
    Q_D(const JourneyReply);
    return d->journeys;
}

std::vector<Journey>&& JourneyReply::takeResult()
{
    Q_D(JourneyReply);
    return std::move(d->journeys);
}

void JourneyReply::addResult(std::vector<Journey> &&res)
{
    Q_D(JourneyReply);
    if (d->journeys.empty()) {
        d->journeys = std::move(res);
    } else {
        d->journeys.insert(d->journeys.end(), res.begin(), res.end());
    }

    d->pendingOps--;
    d->emitFinishedIfDone(this);
}
