/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

    [[nodiscard]] QString name() const;
    void setName(const QString &name);

    [[nodiscard]] QVector<QString> elements() const;
    void setElements(const QVector<QString> &elems);

    [[nodiscard]] QDateTime beginDateTime() const;
    [[nodiscard]] QDateTime endDateTime() const;

    bool load(const QString &path);
    void store(const QString &path) const;

private:
    TripGroupManager *m_mgr = nullptr;
    QString m_name;
    QVector<QString> m_elements;
};

Q_DECLARE_METATYPE(TripGroup)

#endif // TRIPGROUP_H
