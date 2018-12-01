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

#include <KPublicTransport/Journey>
#include <KPublicTransport/Location>
#include <KPublicTransport/NavitiaClient>
#include <KPublicTransport/NavitiaParser>

#include <QCoreApplication>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace KPublicTransport;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QNetworkAccessManager nam;

    Location from;
    from.setCoordinate(2.57110, 49.00406);
    Location to;
    to.setCoordinate(2.37708, 48.84388);
    auto reply = NavitiaClient::findJourney(from, to, QDateTime::currentDateTime(), &nam);
    QObject::connect(reply, &QNetworkReply::finished, [reply, &app]{
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            app.exit();
        } else {
            qDebug() << "Success!";
            auto res = NavitiaParser::parseJourneys(reply);

            for (const auto &journey : res) {
                qDebug() << journey.sections().size();
                for (const auto &section : journey.sections()) {
                    qDebug() << section.departureTime() << section.arrivalTime() << section.route().line().name() << section.route().direction();
                }
            }
            app.exit();
        }
    });

    return QCoreApplication::exec();
}
