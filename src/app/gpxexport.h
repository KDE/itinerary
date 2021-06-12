/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GPXEXPORT_H
#define GPXEXPORT_H

#include <KPublicTransport/Journey>

#include <gpx/gpxwriter.h>

class Transfer;

/** Trip group to GPX export. */
class GpxExport
{
public:
    explicit GpxExport(QIODevice *out);
    ~GpxExport();

    void writeReservation(const QVariant &res, const KPublicTransport::JourneySection &journey = {});
    void writeTransfer(const Transfer &transfer);

    inline void writeStartRoute() { m_writer.writeStartRoute(); }
    inline void writeEndRoute() { m_writer.writeEndRoute(); }

private:
    void writeJourneySection(const KPublicTransport::JourneySection &section);

    Gpx::Writer m_writer;
};

#endif // GPXEXPORT_H
