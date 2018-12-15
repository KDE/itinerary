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

#include "backends/navitiaclient.h"
#include "backends/navitiaparser.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/Location>

#include <QNetworkReply>

using namespace KPublicTransport;

namespace KPublicTransport {
class DepartureReplyPrivate : public ReplyPrivate {
public:
    std::vector<Departure> departures;
};
}

DepartureReply::DepartureReply(const DepartureRequest &req, QNetworkAccessManager *nam)
    : Reply(new DepartureReplyPrivate)
{
    auto reply = NavitiaClient::queryDeparture(req, nam);
    connect(reply, &QNetworkReply::finished, [reply, this] {
        Q_D(DepartureReply);
        switch (reply->error()) {
            case QNetworkReply::NoError:
                d->departures = NavitiaParser::parseDepartures(reply->readAll());
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

DepartureReply::~DepartureReply() = default;

std::vector<Departure> DepartureReply::departures() const
{
    Q_D(const DepartureReply);
    return d->departures; // TODO this copies
}
