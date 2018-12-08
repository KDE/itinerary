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
#include "journeyrequest.h"
#include "logging.h"

#include "backends/navitiaclient.h"
#include "backends/navitiaparser.h"

#include <KPublicTransport/Journey>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QNetworkReply>

using namespace KPublicTransport;

namespace KPublicTransport {
class JourneyReplyPrivate {
public:
    std::vector<Journey> journeys;
    QString errorMsg;
    JourneyReply::Error error = JourneyReply::NoError;
};
}

JourneyReply::JourneyReply(const JourneyRequest &req, QNetworkAccessManager *nam)
    : d(new JourneyReplyPrivate)
{
    auto reply = NavitiaClient::findJourney(req, nam);
    connect(reply, &QNetworkReply::finished, [reply, this] {
        switch (reply->error()) {
            case QNetworkReply::NoError:
                d->journeys = NavitiaParser::parseJourneys(reply->readAll());
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
    // TODO avoid the copy here
    return d->journeys;
}

JourneyReply::Error JourneyReply::error() const
{
    return d->error;
}

QString JourneyReply::errorString() const
{
    return d->errorMsg;
}
