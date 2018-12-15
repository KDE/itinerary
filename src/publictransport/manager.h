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

#ifndef KPUBLICTRANSPORT_MANAGER_H
#define KPUBLICTRANSPORT_MANAGER_H

#include <memory>

class QNetworkAccessManager;

namespace KPublicTransport {

class DepartureReply;
class DepartureRequest;
class JourneyReply;
class JourneyRequest;
class ManagerPrivate;

/** Entry point for starting public transport queries. */
class Manager
{
public:
    Manager();
    Manager(Manager&&) noexcept;
    Manager(const Manager&) = delete;
    ~Manager();

    /** Set the network access manager to use for network operations.
     *  If not set, an instance is created internally.
     */
    void setNetworkAccessManager(QNetworkAccessManager *nam);

    /** Query a journey. */
    JourneyReply* queryJourney(const JourneyRequest &req) const;

    /** Query departures from a specific station. */
    DepartureReply* queryDeparture(const DepartureRequest &req) const;

private:
    std::unique_ptr<ManagerPrivate> d;
};

}

#endif // KPUBLICTRANSPORT_MANAGER_H
