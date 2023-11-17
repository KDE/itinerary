/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itinerarycreator.h"

#include <QImage>
#include <QScopedPointer>

#include <KPkPass/Pass>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(ItineraryCreator, "itinerarythumbnail.json")

ItineraryCreator::ItineraryCreator(QObject *parent, const QVariantList &args)
    : KIO::ThumbnailCreator(parent, args)
{
}

ItineraryCreator::~ItineraryCreator() = default;

KIO::ThumbnailResult ItineraryCreator::create(const KIO::ThumbnailRequest &request)
{
    QScopedPointer<KPkPass::Pass> pass(KPkPass::Pass::fromFile(request.url().toLocalFile(), nullptr));
    if (pass.isNull()) {
        return KIO::ThumbnailResult::fail();
    }

    // See if it has a dedicated thumbnail
    // The thumbnails are typically quite small, so we just pick the largest one
    // rather than taking into account UI scaling
    for (uint dpr = 3; dpr >= 1; --dpr) {
        QImage image = pass->image(QStringLiteral("thumbnail"), dpr);
        if (!image.isNull()) {
            return KIO::ThumbnailResult::pass(image);
        }
    }

    for (const QString &imageName : {QStringLiteral("icon"), QStringLiteral("logo")}) {
        for (uint dpr = 3; dpr >= 1; --dpr) {
            QImage image = pass->image(imageName, dpr);
            if (!image.isNull()) {
                return KIO::ThumbnailResult::pass(image);
            }
        }
    }

    return KIO::ThumbnailResult::fail();
}

#include "itinerarycreator.moc"

#include "moc_itinerarycreator.cpp"
