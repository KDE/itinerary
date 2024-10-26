/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESERVATIONMANAGER_H
#define RESERVATIONMANAGER_H

#include <KItinerary/ExtractorValidator>

#include <QHash>
#include <QObject>
#include <QVariant>

/** Manages JSON-LD reservation data.
 *  This is done on two levels:
 *  - the raw individual reservation elements (one per traveler and per trip)
 *  - reservation batches for multi-traveler trips
 *  Most consumers probably want to work with the multi-traveler batches rather
 *  than the raw elements.
 *  Batches are identified by a reservation id of a random element in that batch,
 *  that means you can directly retrieve reservation data using the batch id too.
 *
 *  Identifiers are QStrings, which is super ugly, but needed for direct consumption
 *  by QML.
 */
class ReservationManager : public QObject
{
    Q_OBJECT
public:
    explicit ReservationManager(QObject *parent = nullptr);
    ~ReservationManager() override;

    Q_INVOKABLE bool isEmpty() const;
    Q_INVOKABLE QVariant reservation(const QString &id) const;

    /** Adds @p res if it's new, or merges it with an existing reservation or reservation batch.
     *  @param resIdHint If set, this is used as identifier for reservations that would need a new
     *  one, assuming it is not already in use. Keep empty by default, only needs to be set when
     *  importing existing data.
     *  @returns The id of the new or existed merged reservation.
     */
    Q_INVOKABLE QString addReservation(const QVariant &res, const QString &resIdHint = {});
    Q_INVOKABLE void updateReservation(const QString &resId, const QVariant &res);
    Q_INVOKABLE void removeReservation(const QString &id);

    /** Adds reservations after post-processing the input.
     *  Same as calling addReservation() with prior post-processing
     *  @returns A list of reservation ids for the extracted elements. Those can be reservation
     *  ids that previously existed, in case the extracted elements could be merged.
     */
    QList<QString> addReservationsWithPostProcessing(const QList<QVariant> &resData);
    Q_INVOKABLE QString addReservationWithPostProcessing(const QVariant &resData);

    const std::vector<QString> &batches() const;
    bool hasBatch(const QString &batchId) const;
    QString batchForReservation(const QString &resId) const;
    Q_INVOKABLE QStringList reservationsForBatch(const QString &batchId) const;
    Q_INVOKABLE void removeBatch(const QString &batchId);

    /** Returns the batch happening prior to @p batchId, if any. */
    QString previousBatch(const QString &batchId) const;
    /** Returns the batch happening after @p batchId, if any. */
    QString nextBatch(const QString &batchId) const;

    /** Validator configured to accept all supported reservation types. */
    [[nodiscard]] static KItinerary::ExtractorValidator validator();

    /** Returns the reservation for which @p res is a partial update. */
    [[nodiscard]] QVariant isPartialUpdate(const QVariant &res) const;

Q_SIGNALS:
    void reservationAdded(const QString &id);
    void reservationChanged(const QString &id);
    void reservationRemoved(const QString &id);

    void batchAdded(const QString &batchId);
    /** This is emitted when elements are added or removed from the batch,
     *  but its content otherwise stays untouched.
     */
    void batchChanged(const QString &batchId);
    /** This is emitted when the batch content changed, but the batching
     *  as such remains the same.
     */
    void batchContentChanged(const QString &batchId);
    /** This is emitted when the reservation with @p oldBatchId was removed and
     *  it has been used to identify a non-empty batch.
     */
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &batchId);

private:
    static QString reservationsBasePath();
    static QString batchesBasePath();
    void storeReservation(const QString &resId, const QVariant &res) const;

    void loadBatches();
    void initialBatchCreate();
    void storeBatch(const QString &batchId) const;
    void storeRemoveBatch(const QString &batchId) const;

    void updateBatch(const QString &resId, const QVariant &newRes, const QVariant &oldRes);
    void removeFromBatch(const QString &resId, const QString &batchId);

    QString makeReservationId(const QString &resIdHint) const;

    QList<QString> applyPartialUpdate(const QVariant &res);

    mutable QHash<QString, QVariant> m_reservations;

    KItinerary::ExtractorValidator m_validator;

    std::vector<QString> m_batches;
    QHash<QString, QStringList> m_batchToResMap; // ### QStringList for direct consumption by QML
    QHash<QString, QString> m_resToBatchMap;
};

#endif // RESERVATIONMANAGER_H
