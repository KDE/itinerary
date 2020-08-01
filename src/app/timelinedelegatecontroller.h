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

#include <KPublicTransport/Stopover>
#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyRequest>

#include <QObject>
#include <QVariant>

#include <chrono>

class QDateTime;
class QJSValue;
class QTimer;

class LiveDataManager;
class ReservationManager;
class TransferManager;

/** C++ side logic for timeline delegates. */
class TimelineDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* reservationManager READ reservationManager WRITE setReservationManager NOTIFY setupChanged)
    Q_PROPERTY(QObject* liveDataManager READ liveDataManager WRITE setLiveDataManager NOTIFY setupChanged)
    Q_PROPERTY(QObject* transferManager READ transferManager WRITE setTransferManager NOTIFY setupChanged)
    Q_PROPERTY(QString batchId READ batchId WRITE setBatchId NOTIFY batchIdChanged)

    Q_PROPERTY(bool isCurrent READ isCurrent NOTIFY currentChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

    /** The location we are in before this element begins.
     *  This is only relevant for future elements, past elements, or elements without a non-current predecessor return nothing here.
     */
    Q_PROPERTY(QVariant previousLocation READ previousLocation NOTIFY previousLocationChanged)

    Q_PROPERTY(KPublicTransport::Stopover arrival READ arrival NOTIFY arrivalChanged)
    Q_PROPERTY(KPublicTransport::Stopover departure READ departure NOTIFY departureChanged)
    Q_PROPERTY(KPublicTransport::JourneySection journey READ journey NOTIFY journeyChanged)

    /** Effective end time, ie. our best knowledge of arriving at the destination (for transit elements), or
     *  ending of the associated event for non-transit elements.
     */
    Q_PROPERTY(QDateTime effectiveEndTime READ effectiveEndTime NOTIFY arrivalChanged)

    /** Returns whether this is a location changing element. */
    Q_PROPERTY(bool isLocationChange READ isLocationChange NOTIFY contentChanged)
    /** Returns whether the current element is a public transport transit element.
     *  That is, a location change where you don't have to navigate to the destination yourself.
     */
    Q_PROPERTY(bool isPublicTransport READ isPublicTransport NOTIFY contentChanged)

    /** A KPublicTransport::JourneyRequest for the current journey.
     *  This includes the current element as well as any immediately connected following elements.
     */
    Q_PROPERTY(KPublicTransport::JourneyRequest journeyRequest READ journeyRequest NOTIFY contentChanged) // TODO technically notification also depends on other elements, so similar to previousLocationChanged

    /** Inbound connection is unlikely to work. */
    Q_PROPERTY(bool connectionWarning READ connectionWarning NOTIFY connectionWarningChanged)

    /** Reservation has been canceled (by user or provider, we usually don't know which). */
    Q_PROPERTY(bool isCanceled READ isCanceled NOTIFY contentChanged)

public:
    TimelineDelegateController(QObject *parent = nullptr);
    ~TimelineDelegateController();

    QObject* reservationManager() const;
    void setReservationManager(QObject *resMgr);
    QObject* liveDataManager() const;
    void setLiveDataManager(QObject *liveDataMgr);
    QObject* transferManager() const;
    void setTransferManager(QObject *transferMgr);

    QString batchId() const;
    void setBatchId(const QString &batchId);

    bool isCurrent() const;
    float progress() const;

    QVariant previousLocation() const;

    KPublicTransport::Stopover arrival() const;
    KPublicTransport::Stopover departure() const;
    KPublicTransport::JourneySection journey() const;

    QDateTime effectiveEndTime() const;

    bool isLocationChange() const;
    bool isPublicTransport() const;

    KPublicTransport::JourneyRequest journeyRequest() const;
    Q_INVOKABLE void applyJourney(const QVariant &journey);

    bool connectionWarning() const;
    bool isCanceled() const;

    /** Map page arguments for the arrival side, if this is a location change element. */
    Q_INVOKABLE QJSValue arrivalMapArguments() const;
    /** Map page arguments for the departure side, if this is a location change element. */
    Q_INVOKABLE QJSValue departureMapArguments() const;

Q_SIGNALS:
    void setupChanged();
    void batchIdChanged();
    void contentChanged();
    void currentChanged();
    void progressChanged();
    void arrivalChanged();
    void departureChanged();
    void journeyChanged();
    void previousLocationChanged();
    void connectionWarningChanged();

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
    TransferManager *m_transferMgr = nullptr;
    QString m_batchId;
    bool m_isCurrent = false;

    static void scheduleNextUpdate(std::chrono::milliseconds ms);
    static QTimer *s_currentTimer;
    static int s_progressRefCount;
    static QTimer *s_progressTimer;
};

#endif // TIMELINEDELEGATECONTROLLER_H
