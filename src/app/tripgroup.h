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

#ifndef TRIPGROUP_H
#define TRIPGROUP_H

#include <QMetaType>
#include <QString>
#include <QVector>

class TripGroupManager;

class QDateTime;

/** Trip group object. */
class TripGroup
{
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QDateTime beginDateTime READ beginDateTime)
    Q_PROPERTY(QDateTime endDateTime READ endDateTime)
public:
    TripGroup();
    TripGroup(TripGroupManager *mgr);
    ~TripGroup();

    QString name() const;
    void setName(const QString &name);

    QVector<QString> elements() const;
    void setElements(const QVector<QString> &elems);

    QDateTime beginDateTime() const;
    QDateTime endDateTime() const;

    bool load(const QString &path);
    void store(const QString &path) const;

private:
    TripGroupManager *m_mgr = nullptr;
    QString m_name;
    QVector<QString> m_elements;
};

Q_DECLARE_METATYPE(TripGroup)

#endif // TRIPGROUP_H
