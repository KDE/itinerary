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

#include <file.h>
#include <boardingpass.h>

#include <QLocale>
#include <QtTest/qtest.h>

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
        std::unique_ptr<KPkPass::File> file(KPkPass::File::fromFile(QLatin1String(SOURCE_DIR "/data/boardingpass.pkpass")));
        QVERIFY(file);

        QCOMPARE(file->logoText(), QLatin1String("Boarding Pass"));

        auto headers = file->headerFields();
        QCOMPARE(headers.size(), 2);
        auto field = headers.at(0);
        QCOMPARE(field.label(), QLatin1String("Sitzplatz"));
        QCOMPARE(field.value(), QLatin1String("10E"));

        auto boardingPass = dynamic_cast<KPkPass::BoardingPass*>(file.get());
        QVERIFY(boardingPass);
        QCOMPARE(boardingPass->transitType(), KPkPass::BoardingPass::Air);

        auto barcodes = file->barcodes();
        QCOMPARE(barcodes.size(), 1);
        auto bc = barcodes.at(0);
        QCOMPARE(bc.format(), KPkPass::Barcode::QR);
        QVERIFY(!bc.message().isEmpty());
    }
};

QTEST_GUILESS_MAIN(PkPassTest)

#include "pkpasstest.moc"
