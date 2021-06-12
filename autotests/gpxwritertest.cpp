/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <gpx/gpxwriter.h>

#include <QBuffer>
#include <QDateTime>
#include <QTimeZone>
#include <QtTest/qtest.h>

class GpxWriterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testWrite()
    {
        QByteArray gpxOut;
        QBuffer buffer(&gpxOut);
        buffer.open(QIODevice::WriteOnly);

        {
            Gpx::Writer writer(&buffer);
            writer.writeStartMetadata();
            writer.writeLink(QStringLiteral("https://apps.kde.org/itinerary"), QStringLiteral("KDE Itinerary"));
            writer.writeEndMetadata();

            writer.writeStartRoute();
            writer.writeName(QStringLiteral("ICE 123 from A to B"));
            writer.writeStartRoutePoint(46.1, 7.8);
            writer.writeTime(QDateTime({1996, 10, 14}, {12, 23}, QTimeZone("Europe/Brussels")));
            writer.writeEndRoutePoint();
            writer.writeEndRoute();
        }

        const char expected[] =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1\">\n"
            "    <metadata>\n"
            "        <link href=\"https://apps.kde.org/itinerary\">\n"
            "            <text>KDE Itinerary</text>\n"
            "        </link>\n"
            "    </metadata>\n"
            "    <rte>\n"
            "        <name>ICE 123 from A to B</name>\n"
            "        <rtept lat=\"46.1\" lon=\"7.8\">\n"
            "            <time>1996-10-14T12:23:00+02:00</time>\n"
            "        </rtept>\n"
            "    </rte>\n"
            "</gpx>\n";

        if (gpxOut != expected) {
            qDebug().noquote() << gpxOut;
        }
        QCOMPARE(gpxOut, expected);
    }
};

QTEST_GUILESS_MAIN(GpxWriterTest)

#include "gpxwritertest.moc"
