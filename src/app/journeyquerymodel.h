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

#include <QAbstractListModel>
#include <QString>

#include <vector>

namespace KPublicTransport {
class Journey;
class Manager;
}

class ReservationManager;

/** Alternative train connection query handling. */
class JourneyQueryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    enum Role {
        JourneyRole = Qt::UserRole
    };

public:
    explicit JourneyQueryModel(QObject *parent = nullptr);
    ~JourneyQueryModel();

    void setReservationManager(ReservationManager *mgr);
    void setPublicTransportManager(KPublicTransport::Manager *mgr);

    Q_INVOKABLE void queryJourney(const QString &batchId);
    Q_INVOKABLE void saveJourney(const QString &batchId, int journeyIndex);

    bool isLoading() const;
    QString errorMessage() const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void loadingChanged();
    void errorMessageChanged();

private:
    ReservationManager *m_resMgr;
    KPublicTransport::Manager *m_ptMgr;
    bool m_isLoading = false;
    QString m_errorMsg;
    std::vector<KPublicTransport::Journey> m_journeys;

};

#endif // JOURNEYQUERYMODEL_H
