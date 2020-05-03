/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#include "context.h"
#include "jnisignature.h"
#include "jnitypes.h"

#include <QtAndroid>

using namespace KAndroidExtras;

QAndroidJniObject Context::getPackageName()
{
    return QtAndroid::androidContext().callObjectMethod("getPackageName", Jni::signature<java::lang::String()>());
}
