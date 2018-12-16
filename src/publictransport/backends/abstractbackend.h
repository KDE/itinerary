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

#ifndef KPUBLICTRANSPORT_ABSTRACTBACKEND_H
#define KPUBLICTRANSPORT_ABSTRACTBACKEND_H

#include "reply.h"

#include <QPolygonF>

class QNetworkAccessManager;

namespace KPublicTransport {

class DepartureReply;
class JourneyReply;
class Location;

/** Abstract base class for transport provider backends. */
class AbstractBackend
{
    Q_GADGET
public:
    AbstractBackend();
    virtual ~AbstractBackend();

    /** Identifer for this backend.
     *  Use e.g. for distinguishing backend-specific cache locations etc.
     */
    QString backendId() const;
    void setBackendId(const QString &id);

    /** Checks if this location has been filtered by the network configuration. */
    bool isLocationExcluded(const Location &loc) const;
    void setGeoFilter(const QPolygonF &poly);

    /** Perform a journey query.
     *  @return @c true if performing an async operation, @c false otherwise.
     */
    virtual bool queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const;

    /** Perform a departure query.
     *  @return @c true if performing an async operation, @c false otherwise.
     */
    virtual bool queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const;

protected:
    /** Helper function to call non-public Reply API. */
    template <typename T, typename ...Args> inline static void addResult(T *reply, Args&&... args)
    {
        reply->addResult(std::forward<Args>(args)...);
    }

    inline static void addError(Reply *reply, Reply::Error error, const QString &errorMsg)
    {
        reply->addError(error, errorMsg);
    }

private:
    Q_DISABLE_COPY(AbstractBackend)
    QString m_backendId;
    QPolygonF m_geoFilter;
};

}

#endif // KPUBLICTRANSPORT_ABSTRACTBACKEND_H
