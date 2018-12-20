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

#ifndef KPUBLICTRANSPORT_HAFASMGATEBACKEND_H
#define KPUBLICTRANSPORT_HAFASMGATEBACKEND_H

#include "abstractbackend.h"
#include "hafasmgateparser.h"

#include <QString>

class QJsonObject;
class QNetworkReply;

namespace KPublicTransport {

/** Backend for the Hafas mgate.exe interface. */
class HafasMgateBackend : public AbstractBackend
{
    Q_GADGET
    Q_PROPERTY(QString endpoint MEMBER m_endpoint)
    Q_PROPERTY(QString aid MEMBER m_aid)
    Q_PROPERTY(QString clientId MEMBER m_clientId)
    Q_PROPERTY(QString clientType MEMBER m_clientType)
    Q_PROPERTY(QString clientVersion MEMBER m_clientVersion)
    Q_PROPERTY(QString clientName MEMBER m_clientName)
    Q_PROPERTY(QString version MEMBER m_version)
    /** Salt for request mic/mac parameters, hex-encoded. */
    Q_PROPERTY(QString micMacSalt WRITE setMicMacSalt)
    /** Salt for the request checksum parameter, hex-encoded. */
    Q_PROPERTY(QString checksumSalt WRITE setChecksumSalt)
    Q_PROPERTY(QJsonObject lineModeMap WRITE setLineModeMap)
    /** Identifier type used for stations. Default is backendId(). */
    Q_PROPERTY(QString locationIdentifierType MEMBER m_locationIdentifierType)
public:
    HafasMgateBackend();
    bool isSecure() const override;
    bool queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const override;
    bool queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const override;
    bool queryLocation(LocationReply *reply, QNetworkAccessManager *nam) const override;

private:
    QNetworkReply* postRequest(const QJsonObject &svcReq, QNetworkAccessManager *nam) const;
    void setMicMacSalt(const QString &salt);
    void setChecksumSalt(const QString &salt);
    void setLineModeMap(const QJsonObject &obj);

    HafasMgateParser m_parser;

    QString m_endpoint;
    QString m_aid;
    QString m_clientId;
    QString m_clientType;
    QString m_clientVersion;
    QString m_clientName;
    QString m_version;
    QByteArray m_micMacSalt;
    QByteArray m_checksumSalt;
    QString m_locationIdentifierType;
};

}

#endif // KPUBLICTRANSPORT_HAFASMGATEBACKEND_H
