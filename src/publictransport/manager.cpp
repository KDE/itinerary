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

#include "manager.h"
#include "departurereply.h"
#include "journeyreply.h"

#include <QNetworkAccessManager>

using namespace KPublicTransport;

namespace KPublicTransport {
class ManagerPrivate {
public:
    QNetworkAccessManager* nam();

    QNetworkAccessManager *m_nam = nullptr;
};
}

QNetworkAccessManager* ManagerPrivate::nam()
{
    if (!m_nam) {
        m_nam = new QNetworkAccessManager;
    }
    return m_nam;
}


Manager::Manager() :
    d(new ManagerPrivate)
{
}

Manager::Manager(Manager&&) noexcept = default;
Manager::~Manager() = default;

void Manager::setNetworkAccessManager(QNetworkAccessManager *nam)
{
    // TODO delete d->nam if we created it ourselves
    d->m_nam = nam;
}

JourneyReply* Manager::findJourney(const JourneyRequest &req) const
{
    return new JourneyReply(req, d->nam());
}

DepartureReply* Manager::queryDeparture(const DepartureRequest &req) const
{
    return new DepartureReply(req, d->nam());
}
