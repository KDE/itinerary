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

#include <barcode.h>
#include <boardingpass.h>
#include <location.h>

#include <QLocale>
#include <QtTest/qtest.h>

#include <cmath>

class PkPassTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
        QLocale::setDefault(QLocale("de_DE"));
    }

    void testBoardingPass()
    {
        std::unique_ptr<KPkPass::Pass> pass(KPkPass::Pass::fromFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QVERIFY(pass);

        QCOMPARE(pass->type(), KPkPass::Pass::BoardingPass);
        QCOMPARE(pass->serialNumber(), QLatin1String("1234"));
        QCOMPARE(pass->description(), QLatin1String("description"));
        QCOMPARE(pass->organizationName(), QLatin1String("KDE"));

        QCOMPARE(pass->logoText(), QLatin1String("Boarding Pass"));
        QCOMPARE(pass->backgroundColor(), QColor(61, 174, 233));
        QCOMPARE(pass->relevantDate(), QDateTime(QDate(2017, 9, 17), QTime(0, 4, 0), Qt::UTC));
        QCOMPARE(pass->expirationDate(), QDateTime(QDate(2017, 9, 18), QTime(0, 0, 36), Qt::UTC));
        QCOMPARE(pass->isVoided(), false);
        QCOMPARE(pass->groupingIdentifier(), QLatin1String(""));

        QCOMPARE(pass->fields().size(), 12);
        auto headers = pass->headerFields();
        QCOMPARE(headers.size(), 2);
        auto field = headers.at(0);
        QCOMPARE(field.label(), QLatin1String("Sitzplatz"));
        QCOMPARE(field.value(), QVariant(QLatin1String("10E")));
        QCOMPARE(field.valueDisplayString(), QLatin1String("10E"));
        QCOMPARE(field.key(), QLatin1String("seat"));
        QCOMPARE(field.changeMessage(), QStringLiteral("Sitzplatznummer geändert in 10E"));

        field = pass->field(QLatin1String("boarding"));
        QCOMPARE(field.key(), QLatin1String("boarding"));
        QCOMPARE(field.value(), QLatin1String("20:25"));

        auto boardingPass = dynamic_cast<KPkPass::BoardingPass*>(pass.get());
        QVERIFY(boardingPass);
        QCOMPARE(boardingPass->transitType(), KPkPass::BoardingPass::Air);

        auto barcodes = pass->barcodes();
        QCOMPARE(barcodes.size(), 1);
        auto bc = barcodes.at(0);
        QCOMPARE(bc.format(), KPkPass::Barcode::QR);
        QVERIFY(!bc.message().isEmpty());
        QVERIFY(bc.alternativeText().isEmpty());
        QCOMPARE(bc.messageEncoding(), QLatin1String("iso-8859-1"));

        const auto locs = pass->locations();
        QCOMPARE(locs.size(), 1);
        const auto loc = locs.at(0);
        QVERIFY(std::isnan(loc.altitude()));
        QCOMPARE((int)loc.latitude(), 47);
        QCOMPARE((int)loc.longitude(), 8);
        QCOMPARE(loc.relevantText(), QLatin1String("LX962 Boarding 20:25"));
    }
};

QTEST_GUILESS_MAIN(PkPassTest)

#include "pkpasstest.moc"
