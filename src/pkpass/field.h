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

#include <memory>

class QJsonObject;

namespace KPkPass {

class Pass;
class PassPrivate;
class FieldPrivate;

/** Field element in a KPkPass::Pass.
 * @see https://developer.apple.com/library/content/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/FieldDictionary.html
 */
class KPKPASS_EXPORT Field
{
    Q_GADGET
    Q_PROPERTY(QString key READ key CONSTANT)
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QVariant value READ value CONSTANT)
    Q_PROPERTY(QString valueDisplayString READ valueDisplayString CONSTANT)
    Q_PROPERTY(QString changeMessage READ changeMessage CONSTANT)

public:
    Field();
    Field(const Field&);
    Field(Field&&);
    ~Field();
    Field& operator=(const Field&);

    /** Field key, unique in the pass but not meant for display. */
    QString key() const;
    /** Localized label for display describing this field. */
    QString label() const;

    /** Value of this field.
     *  This can either be a localized string (most common), a date/time value or a number.
     *  Use this for data extraction, prefer valueDisplayString() for displaying data.
     */
    QVariant value() const;
    /** Value of this field, as a localized string for display.
     *  Use this rather than value() for display.
     */
    QString valueDisplayString() const;

    /** The localized change message for this value. */
    QString changeMessage() const;

    // TODO add textAlignment property
private:
    friend class PassPrivate;
    explicit Field(const QJsonObject &obj, const Pass *pass);

    std::shared_ptr<FieldPrivate> d;
};

}

Q_DECLARE_METATYPE(KPkPass::Field)

#endif // KPKPASS_FIELD_H
