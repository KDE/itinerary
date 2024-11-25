/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUP_H
#define TRIPGROUP_H

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QString>

class TripGroupManager;

/** Trip group object. */
class TripGroup
{
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName)
    /** @c true if the name is auto-assigned, @c false when it is user-specified. */
    Q_PROPERTY(bool automaticName READ hasAutomaticName WRITE setNameIsAutomatic)
    /** @c true if the grouping has been done automatically, @c false if it's user edited. */
    Q_PROPERTY(bool automaticallyGrouped READ isAutomaticallyGrouped WRITE setIsAutomaticallyGrouped)
    Q_PROPERTY(QDateTime beginDateTime READ beginDateTime)
    Q_PROPERTY(QDateTime endDateTime READ endDateTime)
    Q_PROPERTY(QString matrixRoomId READ matrixRoomId WRITE setMatrixRoomId)
    Q_PROPERTY(QString slugName READ slugName)
    Q_PROPERTY(QList<QString> elements READ elements)

    Q_PROPERTY(bool hasEnded READ hasEnded STORED false)

public:
    explicit TripGroup();
    ~TripGroup();

    [[nodiscard]] QString name() const;
    void setName(const QString &name);

    [[nodiscard]] bool hasAutomaticName() const;
    void setNameIsAutomatic(bool automatic);
    [[nodiscard]] bool isAutomaticallyGrouped() const;
    void setIsAutomaticallyGrouped(bool automatic);

    [[nodiscard]] QList<QString> elements() const;
    void setElements(const QList<QString> &elems);

    [[nodiscard]] QDateTime beginDateTime() const;
    void setBeginDateTime(const QDateTime &beginDt);
    [[nodiscard]] QDateTime endDateTime() const;
    void setEndDateTime(const QDateTime &endDt);

    /** Returns @c true if we consider this trip group in the past. */
    [[nodiscard]] bool hasEnded(const QDateTime &now = QDateTime::currentDateTime()) const;

    /** Matrix room used for syncing this trip group. */
    [[nodiscard]] QString matrixRoomId() const;
    void setMatrixRoomId(const QString &roomId);

    /** URL slug variant of the name, for use in file names. */
    [[nodiscard]] QString slugName() const;

    bool load(const QString &path);
    void store(const QString &path) const;

    [[nodiscard]] static QJsonObject toJson(const TripGroup &group);
    [[nodiscard]] static TripGroup fromJson(const QJsonObject &obj);

private:
    QString m_name;
    QList<QString> m_elements;
    QDateTime m_beginDateTime;
    QDateTime m_endDateTime;
    QString m_matrixRoomId;
    bool m_automaticName = true;
    bool m_automaticallyGrouped = true;
};

Q_DECLARE_METATYPE(TripGroup)

#endif // TRIPGROUP_H
