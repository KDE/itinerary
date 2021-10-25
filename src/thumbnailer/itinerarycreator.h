/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ITINERARYCREATOR_H
#define ITINERARYCREATOR_H

#include <kio/thumbcreator.h>

class ItineraryCreator : public ThumbCreator
{
public:
    ItineraryCreator();
    ~ItineraryCreator() override;

    bool create(const QString &path, int width, int height, QImage &image) override;
    Flags flags() const override;
};

#endif // ITINERARYCREATOR_H
