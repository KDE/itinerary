/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JSONIO_H
#define JSONIO_H

class QByteArray;
class QJsonValue;

/** JSON value to/from (binary) representation conversion for persistent storage. */
namespace JsonIO
{
    /** Read JSON value from textual JSON or CBOR binary. */
    QJsonValue read(const QByteArray &data);

    enum OutputFormat {
        CBOR,
        JSON
    };

    /** Write JSON value to JSON or CBOR. */
    QByteArray write(const QJsonValue &value, OutputFormat format = CBOR);

    /** Convert binary data to the requested output format. */
    QByteArray convert(const QByteArray &data, OutputFormat format);
}

#endif // JSONIO_H
