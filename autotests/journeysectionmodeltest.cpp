/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "journeysectionmodel.h"

#include <KPublicTransport/Stopover>

#include <QAbstractItemModelTester>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>
#include <QTimeZone>

class JourneySectionModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testJourneySectionModel()
    {
        JourneySectionModel model;
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {10, 00}, QTimeZone("Europe/Berlin")));
        QAbstractItemModelTester modelTester(&model);

        const auto json = QJsonDocument::fromJson(Test::readFile(QLatin1StringView(SOURCE_DIR "/data/publictransport/db-wifi-journey.json"))).object();
        const auto jny = KPublicTransport::Journey::fromJson(json);
        QVERIFY(!jny.sections().empty());
        model.setJourneySection(jny.sections()[0]);
        QCOMPARE(model.rowCount(), 11);

        QCOMPARE(model.departed(), false);
        QCOMPARE(model.arrived(), false);
        for (auto i = 0; i < model.rowCount(); ++i) {
            auto idx = model.index(i, 0);
            QCOMPARE(model.data(idx, JourneySectionModel::ProgressRole).toFloat(), 0.0f);
            QCOMPARE(model.data(idx, JourneySectionModel::StopoverPassedRole).toBool(), false);
        }

        model.setProperty("showProgress", true);
        auto idx = model.index(3, 0);
        QCOMPARE(idx.data(JourneySectionModel::StopoverRole).value<KPublicTransport::Stopover>().stopPoint().name(), QLatin1StringView("Hannover Hbf"));
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.55102f);

        idx = model.index(4, 0);
        QCOMPARE(idx.data(JourneySectionModel::StopoverRole).value<KPublicTransport::Stopover>().stopPoint().name(), QLatin1StringView("Bielefeld Hbf"));
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), false);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);

        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {10, 15}, QTimeZone("Europe/Berlin")));
        idx = model.index(3, 0);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.857143f);

        idx = model.index(4, 0);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), false);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);

        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {7, 15}, QTimeZone("Europe/Berlin"))); // not departed yet
        idx = model.index(0, 0);
        QCOMPARE(model.departureProgress(), 0.0f);
        QCOMPARE(model.departed(), false);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), false);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {7, 37}, QTimeZone("Europe/Berlin"))); // departed
        QCOMPARE(model.departureProgress(), 0.166667f);
        QCOMPARE(model.departed(), true);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), false);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {7, 41}, QTimeZone("Europe/Berlin"))); // close to arrival
        QCOMPARE(model.departureProgress(), 0.833333f);
        QCOMPARE(model.departed(), true);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), false);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {7, 42}, QTimeZone("Europe/Berlin"))); // arrived
        QCOMPARE(model.departureProgress(), 1.0f);
        QCOMPARE(model.departed(), true);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {7, 46}, QTimeZone("Europe/Berlin"))); // departing
        QCOMPARE(model.departureProgress(), 1.0f);
        QCOMPARE(model.departed(), true);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {7, 50}, QTimeZone("Europe/Berlin"))); // departed
        QCOMPARE(model.departureProgress(), 1.0f);
        QCOMPARE(model.departed(), true);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.25f);

        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {12, 0}, QTimeZone("Europe/Berlin"))); // arriving
        idx = model.index(10, 0);
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), false);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        QCOMPARE(model.arrived(), false);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {12, 3}, QTimeZone("Europe/Berlin"))); // arrived
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.0f);
        QCOMPARE(model.arrived(), false);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {12, 4}, QTimeZone("Europe/Berlin"))); // departed
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.142857f);
        QCOMPARE(model.arrived(), false);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {12, 9}, QTimeZone("Europe/Berlin"))); // arriving
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 0.857143f);
        QCOMPARE(model.arrived(), false);
        model.setCurrentDateTime(QDateTime({2021, 12, 21}, {12, 10}, QTimeZone("Europe/Berlin"))); // arrived
        QCOMPARE(idx.data(JourneySectionModel::StopoverPassedRole).toBool(), true);
        QCOMPARE(idx.data(JourneySectionModel::ProgressRole).toFloat(), 1.0f);
        QCOMPARE(model.arrived(), true);
    }
};

QTEST_GUILESS_MAIN(JourneySectionModelTest)

#include "journeysectionmodeltest.moc"
