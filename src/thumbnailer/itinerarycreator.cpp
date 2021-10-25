/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itinerarycreator.h"

#include <QImage>
#include <QScopedPointer>

#include <KPkPass/Pass>

extern "C"
{
    Q_DECL_EXPORT ThumbCreator *new_creator()
    {
        return new ItineraryCreator;
    }
}

ItineraryCreator::ItineraryCreator() = default;
ItineraryCreator::~ItineraryCreator() = default;

bool ItineraryCreator::create(const QString &path, int width, int height, QImage &image)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    QScopedPointer<KPkPass::Pass> pass(KPkPass::Pass::fromFile(path, nullptr));
    if (pass.isNull()) {
        return false;
    }

    // See if it has a dedicated thumbnail
    // The thumbnails are typically quite small, so we just pick the largest one
    // rather than taking into account UI scaling
    for (uint dpr = 3; dpr >= 1; --dpr) {
        image = pass->image(QStringLiteral("thumbnail"), dpr);
        if (!image.isNull()) {
            return true;
        }
    }

    for (const QString &imageName : {QStringLiteral("icon"), QStringLiteral("logo")}) {
        for (uint dpr = 3; dpr >= 1; --dpr) {
            image = pass->image(imageName, dpr);
            if (!image.isNull()) {
                return true;
            }
        }
    }

    return false;
}

ThumbCreator::Flags ItineraryCreator::flags() const
{
    return None;
}
