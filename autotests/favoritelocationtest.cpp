/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <favoritelocationmodel.h>

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class FavoriteLocationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testFavoriteLocationModel()
    {
        FavoriteLocationModel model;
        QAbstractItemModelTester modelTest(&model);

        while (model.rowCount()) {
            model.removeLocation(0);
        }
        QCOMPARE(model.rowCount(), 0);

        model.appendNewLocation();
        QCOMPARE(model.rowCount(), 1);
        QVERIFY(!model.index(0, 0).data(Qt::DisplayRole).toString().isEmpty());

        QVERIFY(model.setData(model.index(0, 0), QStringLiteral("Test Name"), Qt::DisplayRole));
        QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QLatin1String("Test Name"));

        // verify persistence
        {
            FavoriteLocationModel model2;
            QCOMPARE(model2.rowCount(), 1);
            QCOMPARE(model2.index(0, 0).data(Qt::DisplayRole).toString(), QLatin1String("Test Name"));
        }

        model.removeLocation(0);
        QCOMPARE(model.rowCount(), 0);

        {
            FavoriteLocationModel model2;
            QCOMPARE(model2.rowCount(), 0);
        }
    }
};

QTEST_GUILESS_MAIN(FavoriteLocationTest)

#include "favoritelocationtest.moc"
