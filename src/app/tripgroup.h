/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUP_H
#define TRIPGROUP_H

#include <QList>
#include <QMetaType>
#include <QString>

class TripGroupManager;

class QDateTime;

/** Trip group object. */
class TripGroup
{
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QDateTime beginDateTime READ beginDateTime)
    Q_PROPERTY(QDateTime endDateTime READ endDateTime)
    Q_PROPERTY(QString slugName READ slugName)
public:
    TripGroup();
    TripGroup(TripGroupManager *mgr);
    ~TripGroup();

    [[nodiscard]] QString name() const;
    void setName(const QString &name);

    [[nodiscard]] QList<QString> elements() const;
    void setElements(const QList<QString> &elems);

    [[nodiscard]] QDateTime beginDateTime() const;
    [[nodiscard]] QDateTime endDateTime() const;

    /** URL slug variant of the name, for use in file names. */
    [[nodiscard]] QString slugName() const;

    bool load(const QString &path);
    void store(const QString &path) const;

    [[nodiscard]] static QJsonObject toJson(const TripGroup &group);

private:
    TripGroupManager *m_mgr = nullptr;
    QString m_name;
    QList<QString> m_elements;
};

Q_DECLARE_METATYPE(TripGroup)

#endif // TRIPGROUP_H
