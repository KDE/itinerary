/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GPXEXPORT_H
#define GPXEXPORT_H

#include "gpx/gpxwriter.h"

namespace KPublicTransport {
class JourneySection;
}

class FavoriteLocation;
class Transfer;

/** Trip group to GPX export. */
class GpxExport
{
public:
    explicit GpxExport(QIODevice *out);
    ~GpxExport();

    void writeReservation(const QVariant &res, const KPublicTransport::JourneySection &journey, const Transfer &before, const Transfer &after);
    void writeFavoriteLocation(const FavoriteLocation &fav);

private:
    void writeSelfContainedTransfer(const Transfer &transfer);
    void writeTransfer(const Transfer &transfer);
    void writeJourneySection(const KPublicTransport::JourneySection &section);

    Gpx::Writer m_writer;
};

#endif // GPXEXPORT_H
