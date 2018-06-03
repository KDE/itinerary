/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <countryinformation.h>
#include <pkpassmanager.h>
#include <reservationmanager.h>
#include <timelinemodel.h>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class TimelineModelTest : public QObject
{
    Q_OBJECT
private:
    void clearPasses(PkPassManager *mgr)
    {
        for (const auto id : mgr->passes())
            mgr->removePass(id);
    }

    void clearReservations(ReservationManager *mgr)
    {
        for (const auto id : mgr->reservations()) {
            mgr->removeReservation(id);
        }
    }

private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testModel()
    {
        PkPassManager mgr;
        clearPasses(&mgr);
        ReservationManager resMgr;
        clearReservations(&resMgr);

        resMgr.setPkPassManager(&mgr);
        TimelineModel model;
        model.setReservationManager(&resMgr);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(insertSpy.at(0).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(0).at(2).toInt(), 0);
        QVERIFY(updateSpy.isEmpty());
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);

        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 0);
        QCOMPARE(model.rowCount(), 2);

        clearReservations(&resMgr);
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(model.rowCount(), 1);
    }

    void testNestedElements()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);

        TimelineModel model;
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        resMgr.importReservation(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/haus-randa-v1.json")));
        QCOMPARE(insertSpy.size(), 3);
        QCOMPARE(insertSpy.at(0).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(0).at(2).toInt(), 0);
        QCOMPARE(insertSpy.at(1).at(1).toInt(), 1);
        QCOMPARE(insertSpy.at(1).at(2).toInt(), 1);
        QVERIFY(updateSpy.isEmpty());
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Hotel);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementRangeRole), TimelineModel::RangeBegin);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Hotel);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementRangeRole), TimelineModel::RangeEnd);

        // move end date of a hotel booking: dataChanged on RangeBegin, move (or del/ins) on RangeEnd
        resMgr.importReservation(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/haus-randa-v2.json")));
        QCOMPARE(insertSpy.size(), 4);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 1);
        QCOMPARE(insertSpy.at(2).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(2).at(2).toInt(), 0);
        QCOMPARE(rmSpy.at(0).at(1), 2);
        QCOMPARE(model.rowCount(), 4);

        // delete a split element
        const auto resId = model.data(model.index(1, 0), TimelineModel::ReservationIdRole).toString();
        QVERIFY(!resId.isEmpty());
        resMgr.removeReservation(resId);
        QCOMPARE(rmSpy.size(), 4);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
    }

    void testCountryInfos()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);

        TimelineModel model;
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);

        resMgr.importReservation(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/flight-txl-lhr-sfo.json")));
        QCOMPARE(model.rowCount(), 5); //  2x country info, 2x flights, today marker

        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        auto countryInfo = model.index(0, 0).data(TimelineModel::CountryInformationRole).value<CountryInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Left);
        QCOMPARE(countryInfo.drivingSideDiffers(), true);
        QCOMPARE(countryInfo.powerPlugCompatibility(), CountryInformation::Incompatible);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);

        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        countryInfo = model.index(2, 0).data(TimelineModel::CountryInformationRole).value<CountryInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Right);
        QCOMPARE(countryInfo.drivingSideDiffers(), false);
        QCOMPARE(countryInfo.powerPlugCompatibility(), CountryInformation::Incompatible);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);

        // remove the GB flight should also remove the GB country info
        auto resId = model.index(1, 0).data(TimelineModel::ReservationIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);

        // remove the US flight should also remove the US country info
        resId = model.index(1, 0).data(TimelineModel::ReservationIdRole).toString();
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
    }
};

QTEST_GUILESS_MAIN(TimelineModelTest)

#include "timelinemodeltest.moc"
