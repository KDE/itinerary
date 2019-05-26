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

#ifndef TIMELINEDELEGATECONTROLLER_H
#define TIMELINEDELEGATECONTROLLER_H

#include <KPublicTransport/Departure>

#include <QObject>
#include <QVariant>

#include <chrono>

class QDateTime;
class QTimer;

class LiveDataManager;
class ReservationManager;

/** C++ side logic for timeline delegates. */
class TimelineDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* reservationManager READ reservationManager WRITE setReservationManager NOTIFY setupChanged)
    Q_PROPERTY(QObject* liveDataManager READ liveDataManager WRITE setLiveDataManager NOTIFY setupChanged)
    Q_PROPERTY(QString batchId READ batchId WRITE setBatchId NOTIFY contentChanged)

    Q_PROPERTY(bool isCurrent READ isCurrent NOTIFY currentChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

    Q_PROPERTY(KPublicTransport::Departure arrival READ arrival NOTIFY arrivalChanged)
    Q_PROPERTY(KPublicTransport::Departure departure READ departure NOTIFY departureChanged)

public:
    TimelineDelegateController(QObject *parent = nullptr);
    ~TimelineDelegateController();

    QObject* reservationManager() const;
    void setReservationManager(QObject *resMgr);
    QObject* liveDataManager() const;
    void setLiveDataManager(QObject *liveDataMgr);

    QString batchId() const;
    void setBatchId(const QString &batchId);

    bool isCurrent() const;
    float progress() const;

    KPublicTransport::Departure arrival() const;
    KPublicTransport::Departure departure() const;

Q_SIGNALS:
    void setupChanged();
    void contentChanged();
    void currentChanged();
    void progressChanged();
    void arrivalChanged();
    void departureChanged();

private:
    void setCurrent(bool current, const QVariant &res = {});
    void checkForUpdate(const QString &batchId);
    /** Time at which we consider @p res "current". */
    QDateTime relevantStartDateTime(const QVariant &res) const;
    /** Time at which the event starts/stops based on realtime data. */
    QDateTime liveStartDateTime(const QVariant &res) const;
    QDateTime liveEndDateTime(const QVariant &res) const;

    void batchChanged(const QString &batchId);

    ReservationManager *m_resMgr = nullptr; // ### should this be static?
    LiveDataManager *m_liveDataMgr = nullptr;
    QString m_batchId;
    bool m_isCurrent = false;

    static void scheduleNextUpdate(std::chrono::milliseconds ms);
    static QTimer *s_currentTimer;
    static int s_progressRefCount;
    static QTimer *s_progressTimer;
};

#endif // TIMELINEDELEGATECONTROLLER_H
