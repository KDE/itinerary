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

#ifndef KPUBLICTRANSPORT_NAVITIACLIENT_H
#define KPUBLICTRANSPORT_NAVITIACLIENT_H

class QDateTime;
class QNetworkAccessManager;
class QNetworkReply;

namespace KPublicTransport {

class Location;

/** Navitia REST client methods. */
namespace NavitiaClient
{

// TODO: allow to select if @p dt refers to departure or arrival
QNetworkReply* findJourney(const Location &from, const Location &to, const QDateTime &dt, QNetworkAccessManager *nam);

}

}

#endif // KPUBLICTRANSPORT_NAVITIACLIENT_H
