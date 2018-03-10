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

#ifndef KPKPASS_LOCATION_H
#define KPKPASS_LOCATION_H

#include "kpkpass_export.h"

#include <QMetaType>

#include <memory>

class QJsonObject;

namespace KPkPass {

class LocationPrivate;

/** A pass location element.
 *  @see https://developer.apple.com/library/content/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/LowerLevel.html
 */
class KPKPASS_EXPORT Location
{
    Q_GADGET
    Q_PROPERTY(double altitude READ altitude CONSTANT)
    Q_PROPERTY(double latitude READ latitude CONSTANT)
    Q_PROPERTY(double longitude READ longitude CONSTANT)
    Q_PROPERTY(QString relevantText READ relevantText CONSTANT)
public:
    Location();
    ~Location();

    /** Altitude in meters, NaN if not set. */
    double altitude() const;
    /** Latitude in degree. */
    double latitude() const;
    /** Longitude in degree. */
    double longitude() const;
    /** Text to display when location is reached. */
    QString relevantText() const;
private:
    friend class Pass;
    explicit Location(const QJsonObject &obj);
    std::shared_ptr<LocationPrivate> d;
};

}

Q_DECLARE_METATYPE(KPkPass::Location)

#endif // KPKPASS_LOCATION_H
