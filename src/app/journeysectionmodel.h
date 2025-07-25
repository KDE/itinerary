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

    Q_PROPERTY(float departureProgress READ departureProgress NOTIFY progressChanged)
    Q_PROPERTY(bool departed READ departed NOTIFY progressChanged)
    Q_PROPERTY(bool arrived READ arrived NOTIFY progressChanged)

    // when disabled, progress is always 0 / not passed
    Q_PROPERTY(bool showProgress MEMBER m_showProgress NOTIFY showProgressChanged)
    Q_PROPERTY(int sectionCount READ sectionCount NOTIFY journeySectionChanged)
    Q_PROPERTY(QDateTime currentDateTime READ currentDateTime WRITE setCurrentDateTime)

public:
    explicit JourneySectionModel(QObject *parent = nullptr);
    ~JourneySectionModel();

    KPublicTransport::JourneySection journeySection() const;
    void setJourneySection(const KPublicTransport::JourneySection &section);

    enum Role {
        ProgressRole = Qt::UserRole,
        StopoverRole,
        StopoverPassedRole,
    };

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    float departureProgress() const;
    bool departed() const;
    bool arrived() const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    QDateTime currentDateTime() const;

    int sectionCount() const;

Q_SIGNALS:
    void journeySectionChanged();
    void showProgressChanged();
    void progressChanged();

private:
    KPublicTransport::Stopover stopoverForRow(int row) const;
    [[nodiscard]] float progress(int row) const;
    [[nodiscard]] bool stopoverPassed(int row) const;

    KPublicTransport::JourneySection m_journey;
    QTimer m_updateTimer;

    bool m_showProgress = false;

    QDateTime m_unitTestTime;
};

#endif // JOURNEYSECTIONMODEL_H
