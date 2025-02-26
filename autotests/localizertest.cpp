/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localizer.h"

#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class LocalizerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        qputenv("LANG", "en_US.utf-8");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testTemperatureRange()
    {
        QCOMPARE(Localizer::formatTemperatureRange(20.0, 20.0, false), u"20 °C"_s);
        QCOMPARE(Localizer::formatTemperatureRange(20.0, 22.0, false), u"20 °C / 22 °C"_s);
        QCOMPARE(Localizer::formatTemperatureRange(19.5, 20.4, false), u"20 °C"_s);
        QCOMPARE(Localizer::formatTemperatureRange(19.4, 19.6, false), u"19 °C / 20 °C"_s);
        QCOMPARE(Localizer::formatTemperatureRange(-22.0, 18.0, false), u"-21 °C / 18 °C"_s);

        QCOMPARE(Localizer::formatTemperatureRange(19.5, 20.4, true), u"67 °F / 69 °F"_s);
        QCOMPARE(Localizer::formatTemperatureRange(19.4, 19.6, true), u"67 °F"_s);
        QCOMPARE(Localizer::formatTemperatureRange(-22.0, 18.0, true), u"-7 °F / 64 °F"_s);
    }
};

QTEST_GUILESS_MAIN(LocalizerTest)

#include "localizertest.moc"
