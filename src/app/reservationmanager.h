/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESERVATIONMANAGER_H
#define RESERVATIONMANAGER_H

#include <KItinerary/ExtractorValidator>

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QVariant>

class ReservationManager;

/** Represents a batch of reservations.
 *  That is, a set of reservations (tickets or people) referring to the same incidence.
 */
class ReservationBatch
{
    Q_GADGET // for JSON de/serialization
    Q_PROPERTY(QStringList elements MEMBER m_resIds)
    Q_PROPERTY(QDateTime startDateTime MEMBER m_startDt)
    Q_PROPERTY(QDateTime endDateTime MEMBER m_endDt)

public:
    [[nodiscard]] const QStringList& reservationIds() const;

    /** The equivalent of KItinerary::SortUtil::[start|end]DateTime for the corresponding reservation incidence. */
    [[nodiscard]] QDateTime startDateTime() const;
    [[nodiscard]] QDateTime endDateTime() const;

    /** Equivalent to KItinerary::SortUtil::isBefore. */
    [[nodiscard]] static bool isBefore(const ReservationBatch &lhs, const ReservationBatch &rhs);

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] static ReservationBatch fromJson(const QJsonObject &obj);

private:
    friend class ReservationManager;
    QStringList m_resIds; // ### QStringList for direct consumption by QML
    QDateTime m_startDt;
    QDateTime m_endDt;
};

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
    /** Update existing reservation with post-processing. */
    Q_INVOKABLE void updateReservation(const QString &resId, QVariant res);
    Q_INVOKABLE void removeReservation(const QString &id);

    /** Adds reservations after post-processing the input.
     *  Same as calling addReservation() with prior post-processing
     *  @returns A list of reservation ids for the extracted elements. Those can be reservation
     *  ids that previously existed, in case the extracted elements could be merged.
     */
    QList<QString> addReservationsWithPostProcessing(const QList<QVariant> &resData);
    Q_INVOKABLE QString addReservationWithPostProcessing(const QVariant &resData);

    struct ReservationChange {
        QString id;
        QVariant res;
    };
    /** Update an entire batch, without attempting re-batching. */
    void updateBatch(const std::vector<ReservationChange> &changeset);

    const std::vector<QString> &batches() const;
    bool hasBatch(const QString &batchId) const;
    QString batchForReservation(const QString &resId) const;
    Q_INVOKABLE QStringList reservationsForBatch(const QString &batchId) const;
    Q_INVOKABLE void removeBatch(const QString &batchId);

    /** Retursn the batch meta-data for the given batch id. */
    [[nodiscard]] ReservationBatch batch(const QString &batchId) const;
    /** Returns the batch happening prior to @p batchId, if any. */
    QString previousBatch(const QString &batchId) const;
    /** Returns the batch happening after @p batchId, if any. */
    QString nextBatch(const QString &batchId) const;

    /** Validator configured to accept all supported reservation types. */
    [[nodiscard]] static KItinerary::ExtractorValidator validator();

    /** Returns the reservation for which @p res is a partial update. */
    [[nodiscard]] QVariant isPartialUpdate(const QVariant &res) const;

    /* Storage locations of reservations and batches
     * Do not use directly, apart from special cases like Migrator.
     */
    static QString reservationsBasePath();
    static QString batchesBasePath();

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
    void storeReservation(const QString &resId, const QVariant &res) const;

    struct BatchComparator {
        BatchComparator(const ReservationManager *resMgr)
            : m_resMgr(resMgr)
        {
        }
        [[nodiscard]] bool operator()(const QString &lhs, const QString &rhs) const;
        [[nodiscard]] bool operator()(const QString &lhs, const QDateTime &rhs) const;
        [[nodiscard]] bool operator()(const QDateTime &lhs, const QString &rhs) const;
        const ReservationManager *m_resMgr;
    };

    void loadBatches();
    void initialBatchCreate();
    void populateBatchTimes(ReservationBatch &batch) const;
    static void storeBatch(const QString &batchId, const ReservationBatch &batch);
    static void storeRemoveBatch(const QString &batchId);

    void updateBatch(const QString &resId, const QVariant &newRes, const QVariant &oldRes);
    void removeFromBatch(const QString &resId, const QString &batchId);

    QString makeReservationId(const QString &resIdHint) const;

    QList<QString> applyPartialUpdate(const QVariant &res);

    mutable QHash<QString, QVariant> m_reservations;

    KItinerary::ExtractorValidator m_validator;

    std::vector<QString> m_batches;
    QHash<QString, ReservationBatch> m_batchToResMap;
    QHash<QString, QString> m_resToBatchMap;
};

#endif // RESERVATIONMANAGER_H
