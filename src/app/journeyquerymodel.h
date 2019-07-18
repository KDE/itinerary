/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#ifndef JOURNEYQUERYMODEL_H
#define JOURNEYQUERYMODEL_H

#include <KPublicTransport/JourneyQueryModel>

class ReservationManager;

/** Alternative train connection query handling. */
class JourneyQueryModel : public KPublicTransport::JourneyQueryModel
{
    Q_OBJECT
public:
    explicit JourneyQueryModel(QObject *parent = nullptr);
    ~JourneyQueryModel();

    void setReservationManager(ReservationManager *mgr);

    Q_INVOKABLE void saveJourney(const QString &batchId, int journeyIndex);

private:
    ReservationManager *m_resMgr;
};

#endif // JOURNEYQUERYMODEL_H
