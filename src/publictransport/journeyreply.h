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

#include <QObject>

#include <memory>
#include <vector>

class QNetworkAccessManager;

namespace KPublicTransport {

class Journey;
class JourneyReplyPrivate;
class JourneyRequest;

/** Journey query response. */
class JourneyReply : public QObject
{
    Q_OBJECT
public:
    ~JourneyReply();

    /** Returns the found journeys. */
    std::vector<Journey> journeys() const;

    // TODO error messages
Q_SIGNALS:
    /** Emitted whenever the journey search has been completed. */
    void finished();

private:
    friend class Manager;
    explicit JourneyReply(const JourneyRequest &req, QNetworkAccessManager *nam);
    std::unique_ptr<JourneyReplyPrivate> d;
};

}

#endif // KPUBLICTRANSPORT_JOURNEYREPLY_H
