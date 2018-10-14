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

#ifndef MODELVERIFICATIONPOINT_H
#define MODELVERIFICATIONPOINT_H

#include <QString>

#include <vector>

class QAbstractItemModel;

/** Compares a model state serialized to JSON with a current model state. */
class ModelVerificationPoint
{
public:
    explicit ModelVerificationPoint(const QString &refFile);
    ~ModelVerificationPoint();

    void setRoleFilter(std::vector<int> &&filter);

    bool verify(QAbstractItemModel *model) const;

private:
    QString m_refFile;
    std::vector<int> m_roleFilter;
};

#endif // MODELVERIFICATIONPOINT_H
