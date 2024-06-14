/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GENERICPKPASS_H
#define GENERICPKPASS_H

#include <KItinerary/Datatypes>

class GenericPkPassPrivate;

/** Pseudo-schema.org wrapper for generic/uninterpreted pkpass files. */
class GenericPkPass
{
    KITINERARY_GADGET(GenericPkPass)
    KITINERARY_PROPERTY(QString, name, setName)
    KITINERARY_PROPERTY(QString, pkpassPassTypeIdentifier, setPkpassPassTypeIdentifier)
    KITINERARY_PROPERTY(QString, pkpassSerialNumber, setPkpassSerialNumber)
    KITINERARY_PROPERTY(QDateTime, validUntil, setValidUntil)
    KITINERARY_PROPERTY(QVariantList, subjectOf, setSubjectOf)
private:
    QExplicitlySharedDataPointer<GenericPkPassPrivate> d;
};

Q_DECLARE_METATYPE(GenericPkPass)

#endif // GENERICPKPASS_H
