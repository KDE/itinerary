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

#include "backends/navitiaclient.h"
#include "backends/navitiaparser.h"

#include <KPublicTransport/Journey>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QNetworkReply>
#include <QTimeZone>

using namespace KPublicTransport;

namespace KPublicTransport {
class JourneyReplyPrivate : public ReplyPrivate {
public:
    void postProcessJourneys();

    std::vector<Journey> journeys;
};
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
                    auto dt = section.departureTime();
                    dt.setTimeZone(from.timeZone());
                    section.setDepartureTime(dt);
                }
                if (section.from().timeZone().isValid() && !section.to().timeZone().isValid()) {
                    auto to = section.to();
                    to.setTimeZone(section.from().timeZone());
                    section.setTo(to);
                    auto dt = section.arrivalTime();
                    dt.setTimeZone(to.timeZone());
                    section.setArrivalTime(dt);
                }
            }
        }
        journey.setSections(std::move(sections));
    }
}

JourneyReply::JourneyReply(const JourneyRequest &req, QNetworkAccessManager *nam)
    : Reply(new JourneyReplyPrivate)
{
    auto reply = NavitiaClient::findJourney(req, nam);
    connect(reply, &QNetworkReply::finished, [reply, this] {
        Q_D(JourneyReply);
        switch (reply->error()) {
            case QNetworkReply::NoError:
                d->journeys = NavitiaParser::parseJourneys(reply->readAll());
                d->postProcessJourneys();
                break;
            case QNetworkReply::ContentNotFoundError:
                d->error = NotFoundError;
                d->errorMsg = NavitiaParser::parseErrorMessage(reply->readAll());
                break;
            default:
                d->error = NetworkError;
                d->errorMsg = reply->errorString();
                qCDebug(Log) << reply->error() << reply->errorString();
        }

        emit finished();
        deleteLater();
    });
}

JourneyReply::~JourneyReply() = default;

std::vector<Journey> JourneyReply::journeys() const
{
    Q_D(const JourneyReply);
    // TODO avoid the copy here
    return d->journeys;
}
