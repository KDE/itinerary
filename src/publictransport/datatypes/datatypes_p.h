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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KPUBLICTRANSPORT_DATATYPES_P_H
#define KPUBLICTRANSPORT_DATATYPES_P_H

#include <QSharedData>

#define KPUBLICTRANSPORT_MAKE_GADGET(Class) \
Class::Class() : d(new Class ## Private) {} \
Class::Class(const Class&) = default; \
Class::Class(Class&&) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class&) = default; \
Class& Class::operator=(Class&&) = default;

#endif
