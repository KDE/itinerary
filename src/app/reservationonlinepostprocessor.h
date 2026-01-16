// SPDX-FileCopyrightText: 2025 Jonah Brüchert <jbb@kaidan.im>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QCoroTask>
#include <QObject>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QDateTime>

#include <functional>

class ReservationManager;
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
                                   Settings *settings,
                                   std::function<QNetworkAccessManager *()> namFactory,
                                   QObject *parent = nullptr);

private:
    void handlePendingBatches();
    QCoro::Task<> handleBatchChange(QString batchId);

    [[nodiscard]] QCoro::Task<std::optional<QVariant>> processReservation(QVariant reservation);

    [[nodiscard]] QCoro::Task<std::optional<KItinerary::TrainStation>> processTrainStation(KItinerary::TrainStation station);
    [[nodiscard]] QCoro::Task<std::optional<KItinerary::BusStation>> processBusStation(KItinerary::BusStation station);
    [[nodiscard]] QCoro::Task<std::optional<KItinerary::Airport>> processAirport(KItinerary::Airport airport) const;
    [[nodiscard]] QCoro::Task<std::optional<KItinerary::LodgingBusiness>> processHotel(KItinerary::LodgingBusiness hotel);

    /**
     * @param place
     * @param amenity query phrase for nominatim, e.g "railway station"
     * @return nominatim response
     */
    template <typename T>
    [[nodiscard]] QCoro::Task<QJsonArray> queryNominatim(const T &place, const QString &amenityType, const QString &layer = QStringLiteral("poi,railway"));

    template <typename T>
    bool applyResult(const QJsonArray &results, T &place, const std::vector<QLatin1String> &allowedTags) const;

    ReservationManager *m_resMgr = nullptr;
    Settings *m_settings = nullptr;
    std::function<QNetworkAccessManager *()> m_namFactory;
    QDateTime m_nextNominatimQuery = QDateTime::currentDateTimeUtc();
};
