/*
    Copyright (C) 2018 Nicolas Fella <nicolas.fella@gmx.de>

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

#ifndef ANDROIDBRIGHTNESSBACKEND_H
#define ANDROIDBRIGHTNESSBACKEND_H

#include <QObject>
#include "brightnessmanager.h"


class AndroidBrightnessBackend : public BrightnessBackend
{

public:
    explicit AndroidBrightnessBackend(QObject *parent = nullptr);
    virtual ~AndroidBrightnessBackend();

    virtual void maxBrightness();

};

#endif //  ANDROIDBRIGHTNESSBACKEND_H
