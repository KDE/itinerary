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

#ifndef JOURNEYQUERYCONTROLLER_H
#define JOURNEYQUERYCONTROLLER_H

#include <QObject>
#include <QString>

#include <memory>
#include <vector>

namespace KPublicTransport {
class Journey;
class Manager;
}

class ReservationManager;

/** Alternative train connection query handling. */
class JourneyQueryController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QVariantList journeys READ journeys NOTIFY journeysChanged)

public:
    explicit JourneyQueryController(QObject *parent = nullptr);
    ~JourneyQueryController();

    void setReservationManager(ReservationManager *mgr);

    Q_INVOKABLE void queryJourney(const QString &batchId);

    bool isLoading() const;
    QString errorMessage() const;
    QVariantList journeys() const;

Q_SIGNALS:
    void loadingChanged();
    void errorMessageChanged();
    void journeysChanged();

private:
    ReservationManager *m_resMgr;
    std::unique_ptr<KPublicTransport::Manager> m_ptMgr; // TODO share with LiveDataManager!
    bool m_isLoading = false;
    QString m_errorMsg;
    std::vector<KPublicTransport::Journey> m_journeys;

};

#endif // JOURNEYQUERYCONTROLLER_H
