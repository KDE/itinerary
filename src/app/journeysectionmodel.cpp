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
    connect(this, &JourneySectionModel::departureTrailingSegmentLengthChanged, this, [this]() {
        if (!m_data.empty()) {
            Q_EMIT dataChanged(index(0, 0), index(0, 0));
        }
    });
    connect(this, &JourneySectionModel::arrivalLeadingSegmentLengthChanged, this, [this]() {
        if (!m_data.empty()) {
            Q_EMIT dataChanged(index(rowCount() - 1, 0), index(rowCount() - 1, 0));
        }
    });

    connect(this, &JourneySectionModel::showProgressChanged, this, [this]() {
        if (!m_data.empty()) {
            Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
        }
        Q_EMIT journeySectionChanged();
    });
}

JourneySectionModel::~JourneySectionModel() = default;

KPublicTransport::JourneySection JourneySectionModel::journeySection() const
{
    return m_journey;
}

void JourneySectionModel::setJourneySection(const KPublicTransport::JourneySection& section)
{
    beginResetModel();
    m_journey = section;
    m_data.clear();
    m_data.resize(m_journey.intermediateStops().size());
    endResetModel();
    Q_EMIT journeySectionChanged();
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
        case LeadingSegmentLengthRole:
            return m_data[index.row()].leadingLength;
        case TrailingSegmentLengthtRole:
            return m_data[index.row()].trailingLength;
        case LeadingSegmentProgressRole:
            return leadingProgress(index.row());
        case TrailingSegmentProgressRole:
            return trailingProgress(index.row());
        case StopoverRole:
            return m_journey.intermediateStops()[index.row()];
        case StopoverPassedRole:
            return stopoverPassed(index.row());
    }

    return {};
}

bool JourneySectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const auto length = value.toFloat();
    if (length <= 0.0f) {
        return false;
    }

    qCDebug(Log) << index << value << role;
    switch (role) {
        case LeadingSegmentLengthRole:
            m_data[index.row()].leadingLength = length;
            break;
        case TrailingSegmentLengthtRole:
            m_data[index.row()].trailingLength = length;
            break;
        default:
            return false;
    }
    Q_EMIT dataChanged(index, index);
    if (index.row() > 0) {
        Q_EMIT dataChanged(index.sibling(index.row() - 1, 0), index.sibling(index.row() - 1, 0));
    }
    if (index.row() < rowCount() - 1) {
        Q_EMIT dataChanged(index.sibling(index.row() + 1, 0), index.sibling(index.row() + 1, 0));
    }
    return true;
}

QHash<int, QByteArray> JourneySectionModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(LeadingSegmentLengthRole, "leadingLength");
    n.insert(TrailingSegmentLengthtRole, "trailingLength");
    n.insert(LeadingSegmentProgressRole, "leadingProgress");
    n.insert(TrailingSegmentProgressRole, "trailingProgress");
    n.insert(StopoverRole, "stopover");
    n.insert(StopoverPassedRole, "stopoverPassed");
    return n;
}

float JourneySectionModel::departureTrailingProgress() const
{
    return trailingProgress(-1);
}

float JourneySectionModel::arrivalLeadingProgress() const
{
    return leadingProgress(rowCount());
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

float JourneySectionModel::leadingProgress(int row) const
{
    if (!m_showProgress) {
        return 0.0f;
    }

    const auto now = currentDateTime();
    const auto stop = stopoverForRow(row);
    if (arrivalTime(stop) <= now) {
        qCDebug(Log) << row << stop.stopPoint().name() << "already passed" << arrivalTime(stop);
        return 1.0f;
    }

    const auto prevStop = stopoverForRow(row - 1);
    if (departureTime(prevStop) >= now) {
        qCDebug(Log) << row << stop.stopPoint().name() << "not passed yet";
        return 0.0f;
    }

    const float totalTime = departureTime(prevStop).secsTo(arrivalTime(stop));
    const float progressTime = departureTime(prevStop).secsTo(now);

    const float prevLength = row > 0 ? m_data[row - 1].trailingLength : m_departureTrailingLength;
    const float leadingLength = row >= rowCount() ? m_arrivalLeadingLength : m_data[row].leadingLength;
    const float totalLength = leadingLength + prevLength;

    const float progressLength = totalLength * (progressTime / totalTime);
    qCDebug(Log) << row << stop.stopPoint().name() << totalTime << progressTime << totalLength << progressLength << prevLength << (progressLength < prevLength ? 0.0f : ((progressLength - prevLength) / leadingLength));
    return progressLength < prevLength ? 0.0f : ((progressLength - prevLength) / leadingLength);
}

float JourneySectionModel::trailingProgress(int row) const
{
    if (!m_showProgress) {
        return 0.0f;
    }

    const auto now = currentDateTime();
    const auto stop = stopoverForRow(row);
    if (departureTime(stop) >= now) {
        qCDebug(Log) << row << stop.stopPoint().name()<< "not passed yet";
        return 0.0f;
    }

    const auto nextStop = stopoverForRow(row + 1);
    if (arrivalTime(nextStop) <= now) {
        qCDebug(Log) << row << stop.stopPoint().name()<< "already passed";
        return 1.0f;
    }

    const float totalTime = departureTime(stop).secsTo(arrivalTime(nextStop));
    const float progressTime = departureTime(stop).secsTo(now);

    const float nextLength = row < rowCount() - 1 ? m_data[row + 1].leadingLength : m_arrivalLeadingLength;
    const float trailingLength = row < 0 ? m_departureTrailingLength : m_data[row].trailingLength;
    const float totalLength = trailingLength + nextLength;

    const float progressLength = totalLength * (progressTime / totalTime);
    qCDebug(Log) << row << stop.stopPoint().name()<< totalTime << progressTime << totalLength << progressLength << nextLength << (progressLength > nextLength ? 1.0f : (progressLength / trailingLength));
    return progressLength > trailingLength ? 1.0f : (progressLength / trailingLength);
}

float JourneySectionModel::stopoverPassed(int row) const
{
    if (!m_showProgress) {
        return false;
    }

    const auto now = currentDateTime();
    const auto stop = stopoverForRow(row);
    return arrivalTime(stop) <= now;
}

void JourneySectionModel::setCurrentDateTime(const QDateTime& dt)
{
    m_unitTestTime = dt;
}

QDateTime JourneySectionModel::currentDateTime() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}
