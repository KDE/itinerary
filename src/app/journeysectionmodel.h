/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOURNEYSECTIONMODEL_H
#define JOURNEYSECTIONMODEL_H

#include <KPublicTransport/Journey>

#include <QAbstractItemModel>
#include <QTimer>

#include <vector>

namespace KPublicTransport
{
class Stopover;
}

/** Model containing intermediate stops in a KPublicTransport::JourneySection.
 *  Needed for properly displaying progress information in JourneySectionPage.
 *  This unfortunately very strongly tied to that specific way of displaying a journey section.
 */
class JourneySectionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(KPublicTransport::JourneySection journeySection READ journeySection WRITE setJourneySection NOTIFY journeySectionChanged)

    Q_PROPERTY(float departureTrailingSegmentLength MEMBER m_departureTrailingLength NOTIFY departureTrailingSegmentLengthChanged)
    Q_PROPERTY(float arrivalLeadingSegmentLength MEMBER m_arrivalLeadingLength NOTIFY arrivalLeadingSegmentLengthChanged)

    Q_PROPERTY(float departureTrailingProgress READ departureTrailingProgress NOTIFY journeySectionChanged)
    Q_PROPERTY(float arrivalLeadingProgress READ arrivalLeadingProgress NOTIFY journeySectionChanged)
    Q_PROPERTY(bool departed READ departed NOTIFY journeySectionChanged)
    Q_PROPERTY(bool arrived READ arrived NOTIFY journeySectionChanged)

    // when disabled, progress is always 0 / not passed
    Q_PROPERTY(bool showProgress MEMBER m_showProgress NOTIFY showProgressChanged)
    Q_PROPERTY(int sectionCount READ sectionCount NOTIFY journeySectionChanged)

public:
    explicit JourneySectionModel(QObject *parent = nullptr);
    ~JourneySectionModel();

    KPublicTransport::JourneySection journeySection() const;
    void setJourneySection(const KPublicTransport::JourneySection &section);

    enum Role {
        LeadingSegmentLengthRole = Qt::UserRole,
        TrailingSegmentLengthtRole,
        LeadingSegmentProgressRole,
        TrailingSegmentProgressRole,
        StopoverRole,
        StopoverPassedRole,
    };

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    float departureTrailingProgress() const;
    float arrivalLeadingProgress() const;
    bool departed() const;
    bool arrived() const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    QDateTime currentDateTime() const;

    int sectionCount() const;

Q_SIGNALS:
    void journeySectionChanged();
    void departureTrailingSegmentLengthChanged();
    void arrivalLeadingSegmentLengthChanged();
    void showProgressChanged();

private:
    KPublicTransport::Stopover stopoverForRow(int row) const;
    float leadingProgress(int row) const;
    float trailingProgress(int row) const;
    float stopoverPassed(int row) const;

    KPublicTransport::JourneySection m_journey;
    struct Data {
        float leadingLength = 0.0f;
        float trailingLength = 0.0f;
    };
    std::vector<Data> m_data;
    float m_departureTrailingLength = 0.0f;
    float m_arrivalLeadingLength = 0.0f;
    QTimer m_updateTimer;

    bool m_showProgress = false;

    QDateTime m_unitTestTime;
};

#endif // JOURNEYSECTIONMODEL_H
