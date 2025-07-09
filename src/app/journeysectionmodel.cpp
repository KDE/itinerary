/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "journeysectionmodel.h"
#include "logging.h"

#include <KPublicTransport/Stopover>

#include <QDebug>

JourneySectionModel::JourneySectionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &JourneySectionModel::showProgressChanged, this, [this]() {
        if (!m_journey.intermediateStops().empty()) {
            Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
        }
        Q_EMIT progressChanged();
    });

    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (!m_journey.intermediateStops().empty()) {
            Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
        }
        Q_EMIT progressChanged();
    });
    m_updateTimer.setTimerType(Qt::VeryCoarseTimer);
    m_updateTimer.setInterval(std::chrono::minutes(1));
    m_updateTimer.setSingleShot(false);
}

JourneySectionModel::~JourneySectionModel() = default;

KPublicTransport::JourneySection JourneySectionModel::journeySection() const
{
    return m_journey;
}

int JourneySectionModel::sectionCount() const
{
    return rowCount();
}

void JourneySectionModel::setJourneySection(const KPublicTransport::JourneySection &section)
{
    // is this an update to the current state? if so, try to avoid resetting the model
    if (KPublicTransport::JourneySection::isSame(m_journey, section) && m_journey.intermediateStops().size() == section.intermediateStops().size()) {
        m_journey = section;
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
        Q_EMIT journeySectionChanged();
        return;
    }

    beginResetModel();
    m_journey = section;
    endResetModel();
    Q_EMIT journeySectionChanged();

    if (m_journey.intermediateStops().empty()) {
        m_updateTimer.stop();
    } else {
        m_updateTimer.start();
    }
}

int JourneySectionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_journey.intermediateStops().size();
}

QVariant JourneySectionModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_journey.intermediateStops()[index.row()].stopPoint().name();
    case ProgressRole:
        return progress(index.row());
    case StopoverRole: {
        auto stop = m_journey.intermediateStops()[index.row()];
        if (stop.route().line().mode() == KPublicTransport::Line::Unknown) {
            stop.setRoute(m_journey.route());
        }
        return stop;
    }
    case StopoverPassedRole:
        return stopoverPassed(index.row());
    }

    return {};
}

QHash<int, QByteArray> JourneySectionModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(ProgressRole, "progress");
    n.insert(StopoverRole, "stopover");
    n.insert(StopoverPassedRole, "stopoverPassed");
    return n;
}


float JourneySectionModel::departureProgress() const
{
    return progress(-1);
}


bool JourneySectionModel::departed() const
{
    return stopoverPassed(-1);
}

bool JourneySectionModel::arrived() const
{
    return stopoverPassed(rowCount());
}

KPublicTransport::Stopover JourneySectionModel::stopoverForRow(int row) const
{
    if (row < 0) {
        return m_journey.departure();
    }
    if (row >= 0 && row < rowCount()) {
        return m_journey.intermediateStops()[row];
    }
    return m_journey.arrival();
}

static QDateTime departureTime(const KPublicTransport::Stopover &stop)
{
    return stop.hasExpectedDepartureTime() ? stop.expectedDepartureTime() : stop.scheduledDepartureTime();
}

static QDateTime arrivalTime(const KPublicTransport::Stopover &stop)
{
    if (stop.hasExpectedArrivalTime()) {
        return stop.expectedArrivalTime();
    }
    if (stop.scheduledArrivalTime().isValid()) {
        return stop.scheduledArrivalTime();
    }
    return departureTime(stop);
}


float JourneySectionModel::progress(int row) const
{
    if (!m_showProgress) {
        return 0.0f;
    }

    const auto now = currentDateTime();
    const auto stop = stopoverForRow(row);
    if (departureTime(stop) >= now) {
        qCDebug(Log) << row << stop.stopPoint().name() << "not passed yet";
        return 0.0f;
    }

    const auto nextStop = stopoverForRow(row + 1);
    if (arrivalTime(nextStop) <= now) {
        qCDebug(Log) << row << stop.stopPoint().name() << "already passed";
        return 1.0f;
    }

    const float totalTime = departureTime(stop).secsTo(arrivalTime(nextStop));
    const float progressTime = departureTime(stop).secsTo(now);


    return std::clamp(progressTime / totalTime, 0.0f, 1.0f);
}

bool JourneySectionModel::stopoverPassed(int row) const
{
    if (!m_showProgress) {
        return false;
    }

    const auto now = currentDateTime();
    const auto stop = stopoverForRow(row);
    return arrivalTime(stop) <= now;
}

void JourneySectionModel::setCurrentDateTime(const QDateTime &dt)
{
    m_unitTestTime = dt;
    Q_EMIT dataChanged(index(0), index(rowCount() - 1), {Role::ProgressRole, Role::StopoverPassedRole});
    Q_EMIT journeySectionChanged();
}

QDateTime JourneySectionModel::currentDateTime() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}

#include "moc_journeysectionmodel.cpp"
