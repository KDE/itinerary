/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scamwarningmanager.h"

#include <KItinerary/Place>

#include <QSettings>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class ScamWarningTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QSettings settings;
        settings.remove("ScamWarnings");
    }

    void testAirports()
    {
        QCOMPARE(ScamWarningManager::warnForPlace(QString(), u"trip-id-1"_s), false);

        KItinerary::Airport FRA, HHN, NRN;
        FRA.setIataCode(u"FRA"_s);
        HHN.setIataCode(u"HHN"_s);
        NRN.setIataCode(u"NRN"_s);

        QCOMPARE(ScamWarningManager::warnForPlace(FRA, u"trip-id-1"_s), false);
        QCOMPARE(ScamWarningManager::warnForPlace(HHN, u"trip-id-1"_s), true);
        QCOMPARE(ScamWarningManager::warnForPlace(NRN, u"trip-id-1"_s), true);

        ScamWarningManager::ignorePlaceForTrip(HHN, u"trip-id-1"_s);
        ScamWarningManager::ignorePlacePermanently(NRN);
        QCOMPARE(ScamWarningManager::warnForPlace(HHN, u"trip-id-1"_s), false);
        QCOMPARE(ScamWarningManager::warnForPlace(NRN, u"trip-id-1"_s), false);

        QCOMPARE(ScamWarningManager::warnForPlace(HHN, u"trip-id-2"_s), true);
        QCOMPARE(ScamWarningManager::warnForPlace(NRN, u"trip-id-2"_s), false);

        ScamWarningManager::tripRemoved(u"trip-id-1"_s);
        QCOMPARE(ScamWarningManager::warnForPlace(HHN, u"trip-id-1"_s), true);
        QCOMPARE(ScamWarningManager::warnForPlace(NRN, u"trip-id-2"_s), false);
    }
};

QTEST_GUILESS_MAIN(ScamWarningTest)

#include "scamwarningtest.moc"
