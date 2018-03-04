/*
   Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

#ifndef KPKPASS_PASS_P_H
#define KPKPASS_PASS_P_H

#include <QHash>
#include <QJsonObject>
#include <QString>

#include <memory>

namespace KPkPass {
class PassPrivate {
public:
    /** Content of the pass.json file. */
    QJsonObject data() const;
    /** The pass data structure of the pass.json file. */
    QJsonObject passData() const;
    /** Localized message for the given key. */
    QString message(const QString &key) const;

    void parse();
    bool parseMessages(const QString &lang);

    QVector<Field> fields(const QLatin1String &fieldType, const Pass *q) const;

    static Pass *fromData(std::unique_ptr<QIODevice> device, QObject *parent);

    std::unique_ptr<QIODevice> buffer;
    std::unique_ptr<KZip> zip;
    QJsonObject passObj;
    QHash<QString, QString> messages;
    Pass::Type passType;
};
}

#endif // KPKPASS_PASS_H

