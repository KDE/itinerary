/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-itinerary.h>

#include "matrixcontroller.h"

#if HAVE_MATRIX
#include "matrix/matrixmanager.h"
#include "matrix/matrixroomsmodel.h"
#include "matrix/matrixroomssortproxymodel.h"
#endif

MatrixController::MatrixController(QObject *parent)
    : QObject(parent)
{
#if HAVE_MATRIX
    qRegisterMetaType<MatrixManager *>();
    m_mgr = new MatrixManager(this);
#endif
}

MatrixController::~MatrixController() = default;

bool MatrixController::isAvailable()
{
#if HAVE_MATRIX
    return true;
#else
    return false;
#endif
}

QObject *MatrixController::manager() const
{
#if HAVE_MATRIX
    return m_mgr;
#else
    return nullptr;
#endif
}

QAbstractItemModel *MatrixController::roomsModel()
{
#if HAVE_MATRIX
    if (!m_roomsModel) {
        auto roomsModel = new MatrixRoomsModel(this);
        roomsModel->setConnection(m_mgr->connection());
        connect(m_mgr, &MatrixManager::connectionChanged, roomsModel, [roomsModel, this]() {
            roomsModel->setConnection(m_mgr->connection());
        });

        m_roomsModel = new MatrixRoomsSortProxyModel(this);
        m_roomsModel->setSourceModel(roomsModel);
    }
    return m_roomsModel;
#else
    return nullptr;
#endif
}

#include "moc_matrixcontroller.cpp"
