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

Barcode::Barcode()
{
}

Barcode::Barcode(const QJsonObject &obj, const Pass *pass)
{
    m_altText = pass->d->message(obj.value(QLatin1String("altText")).toString());
    const auto format = obj.value(QLatin1String("format")).toString();
    if (format == QLatin1String("PKBarcodeFormatQR"))
        m_format = QR;
    else if (format == QLatin1String("PKBarcodeFormatPDF417"))
        m_format = PDF417;
    else if (format == QLatin1String("PKBarcodeFormatAztec"))
        m_format = Aztec;
    else if (format == QLatin1String("PKBarcodeFormatCode128"))
        m_format = Code128;
    m_message = obj.value(QLatin1String("message")).toString();
}

Barcode::~Barcode() = default;

QString Barcode::alternativeText() const
{
    return m_altText;
}

Barcode::Format KPkPass::Barcode::format() const
{
    return m_format;
}

QString Barcode::message() const
{
    return m_message;
}
