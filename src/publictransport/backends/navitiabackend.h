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

#ifndef KPUBLICTRANSPORT_NAVITIABACKEND_H
#define KPUBLICTRANSPORT_NAVITIABACKEND_H

#include "abstractbackend.h"

#include <QString>

namespace KPublicTransport {

/** Backend for Navitia-based providers. */
class NavitiaBackend : public AbstractBackend
{
    Q_GADGET
    Q_PROPERTY(QString endpoint MEMBER m_endpoint)
    Q_PROPERTY(QString coverage MEMBER m_coverage)
    Q_PROPERTY(QString authorization MEMBER m_auth)

public:
    NavitiaBackend();

    bool isSecure() const override;
    bool queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const override;
    bool queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const override;

private:
    QString m_endpoint;
    QString m_coverage;
    QString m_auth;
};

}

#endif // KPUBLICTRANSPORT_NAVITIABACKEND_H
