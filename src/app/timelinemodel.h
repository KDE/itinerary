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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TIMELINEMODEL_H
#define TIMELINEMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

class PkPassManager;
class ReservationManager;

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int todayRow READ todayRow NOTIFY todayRowChanged)

public:
    enum Role {
        PassRole = Qt::UserRole + 1,
        PassIdRole,
        SectionHeader,
        ReservationRole,
        ElementTypeRole,
        TodayEmptyRole,
        IsTodayRole
    };

    enum ElementType {
        Flight,
        TrainTrip,
        BusTrip,
        Hotel,
        Restaurant,
        TodayMarker
    };
    Q_ENUM(ElementType)

    // indicates whether an element is self-contained or the beginning/end of a longer timespan/range
    enum RangeType {
        SelfContained,
        RangeBegin,
        RangeEnd
    };
    Q_ENUM(RangeType)

    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel();

    void setPkPassManager(PkPassManager *mgr);
    void setReservationManager(ReservationManager *mgr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int todayRow() const;

signals:
    void todayRowChanged();

private:
    void reservationAdded(const QString &resId);
    void reservationUpdated(const QString &resId);
    void reservationRemoved(const QString &resId);

    PkPassManager *m_passMgr = nullptr;
    ReservationManager *m_resMgr = nullptr;

    struct Element {
        QString id; // reservation id
        QDateTime dt; // relevant date/time
        RangeType rangeType;
    };
    std::vector<Element> m_elements;
};

#endif // TIMELINEMODEL_H
