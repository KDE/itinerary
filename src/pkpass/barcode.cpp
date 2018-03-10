/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "barcode.h"
#include "pass.h"
#include "pass_p.h"

#include <QJsonObject>

using namespace KPkPass;

namespace KPkPass {
class BarcodePrivate
{
public:
    const Pass *pass = nullptr;
    QJsonObject obj;
};
}

Barcode::Barcode()
    : d(new BarcodePrivate)
{
}

Barcode::Barcode(const QJsonObject &obj, const Pass *pass)
    : d(new BarcodePrivate)
{
    d->pass = pass;
    d->obj = obj;
}

Barcode::~Barcode() = default;

QString Barcode::alternativeText() const
{
    if (d->pass) {
        return d->pass->d->message(d->obj.value(QLatin1String("altText")).toString());
    }
    return {};
}

Barcode::Format KPkPass::Barcode::format() const
{
    const auto format = d->obj.value(QLatin1String("format")).toString();
    if (format == QLatin1String("PKBarcodeFormatQR")) {
        return QR;
    } else if (format == QLatin1String("PKBarcodeFormatPDF417")) {
        return PDF417;
    } else if (format == QLatin1String("PKBarcodeFormatAztec")) {
        return Aztec;
    } else if (format == QLatin1String("PKBarcodeFormatCode128")) {
        return Code128;
    }
    return Invalid;
}

QString Barcode::message() const
{
    return d->obj.value(QLatin1String("message")).toString();
}

QString Barcode::messageEncoding() const
{
    return d->obj.value(QLatin1String("messageEncoding")).toString();
}
