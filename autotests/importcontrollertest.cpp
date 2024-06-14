/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"
#include "testhelper.h"

#include "importcontroller.h"
#include "reservationmanager.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include <QAbstractItemModelTester>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class ImportControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testFromUrl()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        QSignalSpy infoMsgSpy(&ctrl, &ImportController::infoMessage);
        ctrl.setReservationManager(&resMgr);
        QCOMPARE(ctrl.hasSelection(), false);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/mixed-reservation-ticket.json")));
        QCOMPARE(ctrl.rowCount(), 2);
        QCOMPARE(ctrl.documents().size(), 0);
        QVERIFY(showImportPageSpy.wait());
        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Reservation);
        QVERIFY(idx.data(ImportController::IconNameRole).value<QString>().contains("train"_L1));
        idx = ctrl.index(1, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Pass);
        QCOMPARE(idx.data(ImportController::BatchSizeRole).value<int>(), 0);
        QCOMPARE(idx.data(ImportController::AttachmentCountRole).value<int>(), 0);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/iata-bcbp-demo.pdf")));
        QCOMPARE(ctrl.rowCount(), 3);
        QCOMPARE(ctrl.documents().size(), 1);
        QCOMPARE(ctrl.pkPasses().size(), 0);
        idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Reservation);
        QVERIFY(idx.data(ImportController::IconNameRole).value<QString>().contains("flight"_L1));
        QVERIFY(!idx.data(ImportController::SubtitleRole).value<QString>().isEmpty());
        QCOMPARE(idx.data(ImportController::BatchSizeRole).value<int>(), 0);
        QCOMPARE(idx.data(ImportController::AttachmentCountRole).value<int>(), 1);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(ctrl.rowCount(), 4);
        QCOMPARE(ctrl.documents().size(), 1);
        QCOMPARE(ctrl.pkPasses().size(), 1);
        QCOMPARE(ctrl.bundles().size(), 0);
        idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Reservation);
        QVERIFY(idx.data(ImportController::TitleRole).value<QString>().contains("ZRH"_L1));
        QCOMPARE(idx.data(ImportController::BatchSizeRole).value<int>(), 0);
        QCOMPARE(idx.data(ImportController::AttachmentCountRole).value<int>(), 1);

        ctrl.clear();
        QCOMPARE(ctrl.rowCount(), 0);
        QCOMPARE(ctrl.documents().size(), 0);
        QCOMPARE(ctrl.pkPasses().size(), 0);
        QCOMPARE(showImportPageSpy.size(), 1);
        QCOMPARE(ctrl.bundles().size(), 0);

        QCOMPARE(infoMsgSpy.size(), 0);
    }

    void testBatching()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        ctrl.setReservationManager(&resMgr);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        QCOMPARE(ctrl.rowCount(), 2);
        QCOMPARE(ctrl.documents().size(), 0);
        QVERIFY(showImportPageSpy.wait());

        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Reservation);
        QCOMPARE(idx.data(ImportController::BatchSizeRole).value<int>(), 1);
        idx = ctrl.index(1, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Reservation);
        QCOMPARE(idx.data(ImportController::BatchSizeRole).value<int>(), 1);
    }

    void testPartialUpdate()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        ctrl.setReservationManager(&resMgr);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/4U8465-cancel.json")));
        QCOMPARE(ctrl.rowCount(), 0);
        QCOMPARE(ctrl.documents().size(), 0);
        QCOMPARE(showImportPageSpy.size(), 0);

        resMgr.addReservation(KItinerary::JsonLdDocument::fromJson(QJsonDocument::fromJson(Test::readFile(QLatin1StringView(SOURCE_DIR "/data/4U8465-v1.json"))).array()).at(0));
        QCOMPARE(resMgr.batches().size(), 1);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/4U8465-cancel.json")));
        QCOMPARE(ctrl.rowCount(), 1);
        QCOMPARE(ctrl.documents().size(), 0);
        QVERIFY(showImportPageSpy.wait());
        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Reservation);
        QCOMPARE(idx.data(ImportController::BatchSizeRole).value<int>(), 0);
    }

    void testBundleImport()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        ctrl.setReservationManager(&resMgr);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/empty-backup.itinerary")));
        QCOMPARE(ctrl.rowCount(), 1);
        QVERIFY(showImportPageSpy.wait());
        QCOMPARE(ctrl.documents().size(), 0);
        QCOMPARE(ctrl.bundles().size(), 1);

        ctrl.clear();
        QCOMPARE(ctrl.rowCount(), 0);
        QCOMPARE(showImportPageSpy.size(), 1);
        QCOMPARE(ctrl.bundles().size(), 0);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/randa2017.itinerary")));
        QCOMPARE(ctrl.rowCount(), 11);
        QVERIFY(showImportPageSpy.wait());
        QCOMPARE(showImportPageSpy.size(), 2);
        QCOMPARE(ctrl.documents().size(), 0);
        QCOMPARE(ctrl.bundles().size(), 1);
    }

    void testCalendarImport()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        ctrl.setReservationManager(&resMgr);

        KCalendarCore::MemoryCalendar::Ptr calendar(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
        KCalendarCore::ICalFormat format;
        QVERIFY(format.load(calendar, QLatin1StringView(SOURCE_DIR "/data/randa2017.ics")));

        QCOMPARE(ctrl.hasSelection(), false);
        ctrl.m_todayOverride = { 2017, 6, 27 };
        ctrl.importFromCalendar(calendar.data());

        QCOMPARE(ctrl.rowCount(), 5);
        QCOMPARE(ctrl.hasSelection(), true);
        QVERIFY(showImportPageSpy.wait());

        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TitleRole).toString(), "Hotel reservation: Haus Randa"_L1);
        QCOMPARE(idx.data(ImportController::IconNameRole).toString(), "meeting-attending"_L1);
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), false);
        QVERIFY(ctrl.setData(idx, true, ImportController::SelectedRole));

        idx = ctrl.index(1, 0);
        QCOMPARE(idx.data(ImportController::TitleRole).toString(), "Train from Visp to Randa"_L1);
        QVERIFY(idx.data(ImportController::IconNameRole).toString().contains(QRegularExpression(u"^qrc:///.*train.*\\.svg$"_s)));
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), true);
        auto res = idx.data(ImportController::DataRole);
        QVERIFY(KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res));
        QVERIFY(ctrl.setData(idx, false, ImportController::SelectedRole));

        idx = ctrl.index(2, 0);
        QCOMPARE(idx.data(ImportController::TitleRole).toString(), "KDE Randa Meeting 2017"_L1);
        QCOMPARE(idx.data(ImportController::IconNameRole).toString(), "meeting-attending"_L1);
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), false);
        res = idx.data(ImportController::DataRole);
        QVERIFY(KItinerary::JsonLd::isA<KItinerary::EventReservation>(res));
        QVERIFY(ctrl.setData(idx, true, ImportController::SelectedRole));

        idx = ctrl.index(3, 0);
        QCOMPARE(idx.data(ImportController::TitleRole).toString(), "Raclette"_L1);
        QCOMPARE(idx.data(ImportController::IconNameRole).toString(), "qrc:///images/foodestablishment.svg"_L1);
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), true);
        res = idx.data(ImportController::DataRole);
        QVERIFY(KItinerary::JsonLd::isA<KItinerary::FoodEstablishmentReservation>(res));
        QVERIFY(ctrl.setData(idx, false, ImportController::SelectedRole));

        idx = ctrl.index(4, 0);
        QCOMPARE(idx.data(ImportController::TitleRole).toString(), "Train from Randa to Visp"_L1);
        QVERIFY(idx.data(ImportController::IconNameRole).toString().contains(QRegularExpression(u"^qrc:///.*train.*\\.svg$"_s)));
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), true);
        res = idx.data(ImportController::DataRole);
        QVERIFY(KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res));
        QVERIFY(ctrl.setData(idx, false, ImportController::SelectedRole));

        QCOMPARE(ctrl.hasSelection(), true);
        idx = ctrl.index(2, 0);
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), true);
        QCOMPARE(ctrl.hasSelection(), true);

        ctrl.clearSelected();
        QCOMPARE(ctrl.rowCount(), 3);
        QCOMPARE(ctrl.hasSelection(), false);
    }

#if HAVE_KHEALTHCERTIFICATE
    void testHealthCertificates()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        ctrl.setReservationManager(&resMgr);

        ctrl.importText(QString::fromUtf8(Test::readFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/full-vaccination.txt"))));
        QCOMPARE(ctrl.rowCount(), 1);
        QCOMPARE(ctrl.documents().size(), 0);
        QVERIFY(showImportPageSpy.wait());
        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::HealthCertificate);
        QVERIFY(idx.data(ImportController::IconNameRole).value<QString>().contains("cross"_L1));
        QVERIFY(!idx.data(ImportController::TitleRole).value<QString>().isEmpty());
        QVERIFY(!idx.data(ImportController::SubtitleRole).value<QString>().isEmpty());

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/negative-pcr-test-fr.pdf")));
        QCOMPARE(ctrl.rowCount(), 2);
        QCOMPARE(ctrl.documents().size(), 0); // we don't have support for attached documents for health certificates

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/partial-vaccination.divoc")));
        QCOMPARE(ctrl.rowCount(), 3);

        QCOMPARE(ctrl.documents().size(), 0);
        QCOMPARE(ctrl.pkPasses().size(), 0);
        QCOMPARE(showImportPageSpy.size(), 1);
    }
#endif

    void testNoImport()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        QSignalSpy infoMsgSpy(&ctrl, &ImportController::infoMessage);
        ctrl.setReservationManager(&resMgr);

        ctrl.importData("this is not a valid ticket");

        QCOMPARE(ctrl.rowCount(), 0);
        QCOMPARE(ctrl.hasSelection(), false);
        QCOMPARE(showImportPageSpy.size(), 0);
        QCOMPARE(infoMsgSpy.size(), 1);

        KCalendarCore::MemoryCalendar cal("UTC");
        ctrl.importFromCalendar(&cal);
        QCOMPARE(ctrl.rowCount(), 0);
        QCOMPARE(showImportPageSpy.size(), 0);
        QCOMPARE(infoMsgSpy.size(), 2);
    }

    void testEventTemplate()
    {
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        ctrl.importData("{\"@context\": \"http://schema.org\", \"@type\": \"Event\", \"location\": { \"@type\": \"Place\", \"address\": { \"@type\": \"PostalAddress\", \"addressCountry\": \"CH\", \"addressLocality\": \"Randa\", \"addressRegion\": \"Wallis\", \"postalCode\": \"3928\", \"streetAddress\": \"Haus Maria am Weg\" }, \"geo\": { \"@type\": \"GeoCoordinates\", \"latitude\": 46.09903, \"longitude\": 7.78326 }, \"name\": \"Haus Randa\"} }");

        QCOMPARE(ctrl.rowCount(), 1);
        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TypeRole).value<ImportElement::Type>(), ImportElement::Template);
        QCOMPARE(idx.data(ImportController::IconNameRole).value<QString>(), "meeting-attending"_L1);
        QCOMPARE(idx.data(ImportController::TitleRole).value<QString>(), "Haus Randa"_L1);
    }

    void testGenericPass()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        ImportController ctrl;
        QAbstractItemModelTester modelTest(&ctrl);
        QSignalSpy showImportPageSpy(&ctrl, &ImportController::showImportPage);
        QSignalSpy infoMsgSpy(&ctrl, &ImportController::infoMessage);
        ctrl.setReservationManager(&resMgr);

        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/generic-pass.pkpass")));

        QCOMPARE(ctrl.rowCount(), 1);
        QCOMPARE(ctrl.hasSelection(), true);
        QVERIFY(showImportPageSpy.wait());
        QCOMPARE(infoMsgSpy.size(), 0);

        auto idx = ctrl.index(0, 0);
        QCOMPARE(idx.data(ImportController::TitleRole).toString(), "KDE"_L1);
        QCOMPARE(idx.data(ImportController::IconNameRole).toString(), "bookmarks"_L1);
        QCOMPARE(idx.data(ImportController::SelectedRole).toBool(), true);
        QCOMPARE(idx.data(ImportController::AttachmentCountRole).toInt(), 1);
    }
};

QTEST_GUILESS_MAIN(ImportControllerTest)

#include "importcontrollertest.moc"
