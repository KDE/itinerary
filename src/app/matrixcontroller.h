/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXCONTROLLER_H
#define MATRIXCONTROLLER_H

#include <QObject>
#include <QAbstractItemModel>
class MatrixManager;
class MatrixRoomsModel;
class MatrixRoomsSortProxyModel;

/** Itinerary-specific Matrix integration logic. */
class MatrixController : public QObject
{
    Q_OBJECT
    /** Matrix support is built in at all? */
    Q_PROPERTY(bool isAvailable READ isAvailable CONSTANT)
    Q_PROPERTY(QObject *manager READ manager CONSTANT)
    /** Matrix rooms model. */
    Q_PROPERTY(QAbstractItemModel* roomsModel READ roomsModel CONSTANT)

public:
    explicit MatrixController(QObject *parent = nullptr);
    ~MatrixController();

    static bool isAvailable();

    QObject* manager() const;
    QAbstractItemModel* roomsModel();

private:
    MatrixManager *m_mgr = nullptr;
    MatrixRoomsSortProxyModel *m_roomsModel = nullptr;
};

#endif // MATRIXCONTROLLER_H
