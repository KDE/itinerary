/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-itinerary.h>

#include "matrixcontroller.h"

#if HAVE_MATRIX
#include <kmatrix/matrixmanager.h>
#include <kmatrix/matrixroomsmodel.h>
#endif

MatrixController::MatrixController(QObject *parent)
    : QObject(parent)
{
#if HAVE_MATRIX
    qRegisterMetaType<MatrixManager*>();
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

QObject* MatrixController::manager() const
{
#if HAVE_MATRIX
    return m_mgr;
#else
    return nullptr;
#endif
}

QAbstractItemModel* MatrixController::roomsModel()
{
#if HAVE_MATRIX
    if (!m_roomsModel) {
        m_roomsModel = new MatrixRoomsModel(this);
        m_roomsModel->setConnection(m_mgr->connection());
        connect(m_mgr, &MatrixManager::connectionChanged, m_roomsModel, [this]() { m_roomsModel->setConnection(m_mgr->connection()); });
    }
    return m_roomsModel;
#else
    return nullptr;
#endif
}
