/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericpkpass.h"

#include <QDateTime>

class GenericPkPassPrivate : public QSharedData
{
public:
    QString name;
    QString pkpassPassTypeIdentifier;
    QString pkpassSerialNumber;
    QDateTime validUntil;
};

// TODO replace this by the use of the KITINERARY_XXX implementation macros,
// once those are installed
GenericPkPass::GenericPkPass() : d(new GenericPkPassPrivate) {}
GenericPkPass::GenericPkPass(const GenericPkPass&) = default;
GenericPkPass::~GenericPkPass() = default;
GenericPkPass& GenericPkPass::operator=(const GenericPkPass &other) = default;
QString GenericPkPass::className() const { return QStringLiteral("GenericPkPass"); }
GenericPkPass::operator QVariant() const { return QVariant::fromValue(*this); }
const char* GenericPkPass::typeName() { return "GenericPkPass"; }

QString GenericPkPass::name() const
{
    return d->name;
}

void GenericPkPass::setName(const QString &value)
{
    d.detach();
    d->name = value;
}

QString GenericPkPass::pkpassPassTypeIdentifier() const
{
    return d->pkpassPassTypeIdentifier;
}

void GenericPkPass::setPkpassPassTypeIdentifier(const QString &value)
{
    d.detach();
    d->pkpassPassTypeIdentifier = value;
}

QString GenericPkPass::pkpassSerialNumber() const
{
    return d->pkpassSerialNumber;
}

void GenericPkPass::setPkpassSerialNumber(const QString &value)
{
    d.detach();
    d->pkpassSerialNumber = value;
}

QDateTime GenericPkPass::validUnitl() const
{
    return d->validUntil;
}

void GenericPkPass::setValidUntil(const QDateTime &value)
{
    d.detach();
    d->validUntil = value;
}

bool GenericPkPass::operator==(const GenericPkPass &other) const
{
    return d->pkpassPassTypeIdentifier == other.pkpassPassTypeIdentifier() && d->pkpassSerialNumber == other.pkpassSerialNumber() && d->validUntil == other.validUnitl();
}

#include "moc_genericpkpass.cpp"
