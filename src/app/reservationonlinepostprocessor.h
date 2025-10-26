// SPDX-FileCopyrightText: 2025 Jonah Brüchert <jbb@kaidan.im>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QCoroTask>
#include <QObject>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <functional>

class ReservationManager;
class LiveDataManager;
class QNetworkAccessManager;
class Settings;

/**
 * Adds information to reservations from online sources
 */
class ReservationOnlinePostprocessor : public QObject
{
    Q_OBJECT

public:
    ReservationOnlinePostprocessor(ReservationManager *reservationMgr,
                                   LiveDataManager *liveDataMgr,
                                   Settings *settings,
                                   const std::function<QNetworkAccessManager *()> &namFactory);

private:
    QCoro::Task<> handleReservationChange(const QString id);

    QCoro::Task<QVariant> processReservation(QVariant reservation);

    QCoro::Task<KItinerary::TrainStation> processTrainStation(KItinerary::TrainStation station);
    QCoro::Task<KItinerary::BusStation> processBusStation(KItinerary::BusStation station);

    /**
     * @param place
     * @param amenity query phrase for nominatim, e.g "railway station"
     * @return nominatim response
     */
    QCoro::Task<QJsonArray> queryNominatim(const KItinerary::Place &place,
                                           const QString &amenityType);

    void applyResult(const QJsonArray &results,
                     KItinerary::Place &place,
                     const std::vector<QLatin1String> &allowedTags);

    ReservationManager *m_resMgr = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    Settings *m_settings = nullptr;
    const std::function<QNetworkAccessManager *()> &m_namFactory;
};
