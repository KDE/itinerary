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

#ifndef KPKPASS_BARCODE_H
#define KPKPASS_BARCODE_H

#include "kpkpass_export.h"

#include <QMetaType>
#include <QString>

#include <memory>

class QJsonObject;

namespace KPkPass {

class BarcodePrivate;
class Pass;

/** A pass barcode element.
 *  @see https://developer.apple.com/library/content/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/LowerLevel.html
 */
class KPKPASS_EXPORT Barcode
{
    Q_GADGET
    Q_PROPERTY(QString alternativeText READ alternativeText CONSTANT)
    Q_PROPERTY(Format format READ format CONSTANT)
    Q_PROPERTY(QString message READ message CONSTANT)

public:
    enum Format {
        Invalid,
        QR,
        PDF417,
        Aztec,
        Code128
    };
    Q_ENUM(Format)

    Barcode();
    ~Barcode();

    /** A human readable version of the barcode data. */
    QString alternativeText() const;
    /** The barcode type. */
    Format format() const;
    /** The message encoded in the barcode. */
    QString message() const;

    // TODO add codec property
private:
    friend class Pass;
    explicit Barcode(const QJsonObject &obj, const Pass *file);
    std::shared_ptr<BarcodePrivate> d;
};

}

Q_DECLARE_METATYPE(KPkPass::Barcode)

#endif // KPKPASS_BARCODE_H
