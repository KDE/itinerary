/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpxwriter.h"

#include <QDateTime>

Gpx::Writer::Writer(QIODevice* device)
    : m_writer(device)
{
    m_writer.setAutoFormatting(true);
    m_writer.writeStartDocument(QStringLiteral("1.0"));
    m_writer.writeStartElement(QStringLiteral("gpx"));
    m_writer.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://www.topografix.com/GPX/1/1"));
    m_writer.writeNamespace(QStringLiteral("http://www.w3.org/2001/XMLSchema-instance"), QStringLiteral("xsi"));
    m_writer.writeAttribute(QStringLiteral("xsi:schemaLocation"), QStringLiteral("http://www.topografix.com/GPX/1/1"));
}

Gpx::Writer::~Writer()
{
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
}

void Gpx::Writer::writeStartMetadata()
{
    m_writer.writeStartElement(QStringLiteral("metadata"));
}

void Gpx::Writer::writeEndMetadata()
{
    m_writer.writeEndElement();
}

void Gpx::Writer::writeStartRoute()
{
    m_writer.writeStartElement(QStringLiteral("rte"));
}

void Gpx::Writer::writeEndRoute()
{
    m_writer.writeEndElement();
}

void Gpx::Writer::writeStartWaypoint(float latitude, float longitude)
{
    m_writer.writeStartElement(QStringLiteral("wpt"));
    m_writer.writeAttribute(QStringLiteral("lat"), QString::number(latitude));
    m_writer.writeAttribute(QStringLiteral("lon"), QString::number(longitude));
}

void Gpx::Writer::writeEndWaypoint()
{
    m_writer.writeEndElement();
}

void Gpx::Writer::writeStartRoutePoint(float latitude, float longitude)
{
    m_writer.writeStartElement(QStringLiteral("rtept"));
    m_writer.writeAttribute(QStringLiteral("lat"), QString::number(latitude));
    m_writer.writeAttribute(QStringLiteral("lon"), QString::number(longitude));
}

void Gpx::Writer::writeEndRoutePoint()
{
    m_writer.writeEndElement();
}

void Gpx::Writer::writeName(const QString& name)
{
    m_writer.writeTextElement(QStringLiteral("name"), name);
}

void Gpx::Writer::writeLink(const QString &href, const QString &text)
{
    m_writer.writeStartElement(QStringLiteral("link"));
    m_writer.writeAttribute(QStringLiteral("href"), href);
    if (!text.isEmpty()) {
        m_writer.writeTextElement(QStringLiteral("text"), text);
    }
    m_writer.writeEndElement();
}

void Gpx::Writer::writeTime(const QDateTime &time)
{
    m_writer.writeTextElement(QStringLiteral("time"), time.toString(Qt::ISODate));
}
