/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ITINERARYCREATOR_H
#define ITINERARYCREATOR_H

#include <QObject>

#include <KIO/ThumbnailCreator>

class ItineraryCreator : public KIO::ThumbnailCreator
{
    Q_OBJECT
public:
    ItineraryCreator(QObject *parent, const QVariantList &args);
    ~ItineraryCreator() override;

    KIO::ThumbnailResult create(const KIO::ThumbnailRequest &request) override;
};

#endif // ITINERARYCREATOR_H
