/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ITINERARYCREATOR_H
#define ITINERARYCREATOR_H

#include <KFileMetaData/ExtractorPlugin>

namespace KFileMetaData
{

class ItineraryExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin" FILE "itineraryextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit ItineraryExtractor(QObject *parent = nullptr);

public:
    void extract(ExtractionResult *result) override;
    QStringList mimetypes() const override;
};

}

#endif // ITINERARYCREATOR_H
