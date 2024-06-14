/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericpkpass.h"

#include <kitinerary/datatypes_impl.h>

#include <QDateTime>

class GenericPkPassPrivate : public QSharedData
{
public:
    QString name;
    QString pkpassPassTypeIdentifier;
    QString pkpassSerialNumber;
    QDateTime validUntil;
    QVariantList subjectOf;
};

KITINERARY_MAKE_CLASS(GenericPkPass)
KITINERARY_MAKE_PROPERTY(GenericPkPass, QString, name, setName)
KITINERARY_MAKE_PROPERTY(GenericPkPass, QString, pkpassPassTypeIdentifier, setPkpassPassTypeIdentifier)
KITINERARY_MAKE_PROPERTY(GenericPkPass, QString, pkpassSerialNumber, setPkpassSerialNumber)
KITINERARY_MAKE_PROPERTY(GenericPkPass, QDateTime, validUntil, setValidUntil)
KITINERARY_MAKE_PROPERTY(GenericPkPass, QVariantList, subjectOf, setSubjectOf)
KITINERARY_MAKE_OPERATOR(GenericPkPass)

#include "moc_genericpkpass.cpp"
