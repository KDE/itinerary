/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GPX_WRITER_H
#define GPX_WRITER_H

#include <QXmlStreamWriter>

class QDateTime;
class QIODevice;

namespace Gpx {

/** GPX file writer.
 *  @see https://www.topografix.com/GPX/1/1/
 */
class Writer
{
public:
    explicit Writer(QIODevice *device);
    ~Writer();

    void writeStartMetadata();
    void writeEndMetadata();

    void writeStartRoute();
    void writeEndRoute();

    void writeStartRoutePoint(float latitude, float longitude);
    void writeEndRoutePoint();

    void writeName(const QString &name);
    void writeLink(const QString &href, const QString &text);
    void writeTime(const QDateTime &time);

private:
    QXmlStreamWriter m_writer;
};

}

#endif // GPX_WRITER_H
