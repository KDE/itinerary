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

#include "contenttypeprober.h"

#include <QByteArray>
#include <QUrl>

ContentTypeProber::Type ContentTypeProber::probe(const QByteArray &head, const QUrl &url)
{
    // check content
    if (head.size() >= 4) {
        if (head[0] == 'P' && head[1] == 'K' && head[2] == 0x03 && head[3] == 0x04)
            return PkPass;
        if (head[0] == '%' && head[1] == 'P' && head[2] == 'D' && head[3] == 'F')
            return PDF;
        if (head[0] == '[' || head[0] == '{')
            return JsonLd;
        if (head[0] == 'M' && head[1] >= '1' && head[1] <= '9')
            return IataBcbp;
    }

    // guess from file extension
    const auto fn = url.fileName();
    if (fn.endsWith(QLatin1String(".pkpass"), Qt::CaseInsensitive))
        return PkPass;
    if (fn.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
        return PDF;
    if (fn.endsWith(QLatin1String(".json"), Qt::CaseInsensitive) || fn.endsWith(QLatin1String(".jsonld"), Qt::CaseInsensitive))
        return JsonLd;

    return Unknown;
}
