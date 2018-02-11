/*
   Copyright (c) 2017-2018 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef KPKPASS_FIELD_H
#define KPKPASS_FIELD_H

#include "kpkpass_export.h"

#include <QMetaType>
#include <QString>

class QJsonObject;

namespace KPkPass {

class File;

/** Field element in a KPkPass::File. */
class KPKPASS_EXPORT Field
{
    Q_GADGET
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QString value READ value CONSTANT)
public:
    Field() = default;
    explicit Field(const QJsonObject &obj, const File *file);
    ~Field() = default;

    QString label() const;
    QString value() const;

private:
    QString m_label;
    QString m_value;
};

}

Q_DECLARE_METATYPE(KPkPass::Field)

#endif // KPKPASS_FIELD_H
