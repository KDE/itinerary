// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later


#ifndef QMLSINGLETONS_H
#define QMLSINGLETONS_H

#include "documentmanager.h"
#include "favoritelocationmodel.h"
#include "importcontroller.h"
#include "livedatamanager.h"
#include "mapdownloadmanager.h"
#include "matrixcontroller.h"
#include "passmanager.h"
#include "pkpassmanager.h"
#include "qmlregistrationhelper.h"
#include "reservationmanager.h"
#include "settings.h"
#include "traewellingcontroller.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"
#include "tripgroupmodel.h"
#include "accounts/accountmodel.h"

struct Dummy { // HACK to convince CMake to run moc on this
    Q_GADGET
};

REGISTER_SINGLETON_INSTANCE(ReservationManager)
REGISTER_SINGLETON_INSTANCE(DocumentManager)
REGISTER_SINGLETON_INSTANCE(FavoriteLocationModel)
REGISTER_SINGLETON_INSTANCE(PkPassManager)
REGISTER_SINGLETON_INSTANCE(Settings)
REGISTER_SINGLETON_INSTANCE(TransferManager)
REGISTER_SINGLETON_INSTANCE(TripGroupManager)
REGISTER_SINGLETON_INSTANCE(LiveDataManager)
REGISTER_SINGLETON_INSTANCE(MapDownloadManager)
REGISTER_SINGLETON_INSTANCE(PassManager)
REGISTER_SINGLETON_INSTANCE(ImportController)
REGISTER_SINGLETON_INSTANCE(TripGroupModel)
REGISTER_SINGLETON_INSTANCE(TraewellingController)
REGISTER_SINGLETON_INSTANCE(MatrixController)
REGISTER_SINGLETON_INSTANCE(AccountModel)

#endif
