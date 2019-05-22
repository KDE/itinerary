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

#include <timelinedelegatecontroller.h>
#include <reservationmanager.h>

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KItinerary;

class TimelineDelegateControllerTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        const auto batches = mgr->batches(); // copy, as this is getting modified in the process
        for (const auto &id : batches) {
            mgr->removeBatch(id);
        }
        QCOMPARE(mgr->batches().size(), 0);
    }

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testEmptyController()
    {
        TimelineDelegateController controller;
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);

        controller.setBatchId(QStringLiteral("foo"));
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);

        ReservationManager mgr;
        controller.setReservationManager(&mgr);
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);
    }

    void testController()
    {
        ReservationManager mgr;
        clearReservations(&mgr);

        TrainTrip trip;
        trip.setTrainNumber(QStringLiteral("TGV 1235"));
        trip.setDepartureTime(QDateTime::currentDateTime().addDays(-1));
        TrainReservation res;
        res.setReservationNumber(QStringLiteral("XXX007"));
        res.setReservationFor(trip);

        TimelineDelegateController controller;
        QSignalSpy currentSpy(&controller, &TimelineDelegateController::currentChanged);
        controller.setReservationManager(&mgr);

        mgr.addReservation(res);
        QCOMPARE(mgr.batches().size(), 1);
        const auto batchId = mgr.batches().at(0);

        controller.setBatchId(batchId);
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);

        trip.setArrivalTime(QDateTime::currentDateTime().addDays(1));
        res.setReservationFor(trip);
        mgr.updateReservation(batchId, res);

        QCOMPARE(controller.isCurrent(), true);
        QCOMPARE(controller.progress(), 0.5f);
        QCOMPARE(currentSpy.size(), 1);
    }
};

QTEST_GUILESS_MAIN(TimelineDelegateControllerTest)

#include "timelinedelegatecontrollertest.moc"
