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

#include <QObject>
#include <QVariant>

#include <chrono>

class QTimer;

class ReservationManager;

/** C++ side logic for timeline delegates. */
class TimelineDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* reservationManager READ reservationManager WRITE setReservationManager NOTIFY setupChanged)
    Q_PROPERTY(QString batchId READ batchId WRITE setBatchId NOTIFY contentChanged)

    Q_PROPERTY(bool isCurrent READ isCurrent NOTIFY currentChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

public:
    TimelineDelegateController(QObject *parent = nullptr);
    ~TimelineDelegateController();

    QObject *reservationManager() const;
    void setReservationManager(QObject *resMgr);

    QString batchId() const;
    void setBatchId(const QString &batchId);

    bool isCurrent() const;
    float progress() const;

Q_SIGNALS:
    void setupChanged();
    void contentChanged();
    void currentChanged();
    void progressChanged();

private:
    void setCurrent(bool current, const QVariant &res = {});
    void checkForUpdate(const QString &batchId);

    ReservationManager *m_resMgr = nullptr; // ### should this be static?
    QString m_batchId;
    bool m_isCurrent = false;

    static void scheduleNextUpdate(std::chrono::milliseconds ms);
    static QTimer *s_currentTimer;
    static int s_progressRefCount;
    static QTimer *s_progressTimer;
};

#endif // TIMELINEDELEGATECONTROLLER_H
