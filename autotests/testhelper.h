/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <documentmanager.h>
#include <pkpassmanager.h>
#include <reservationmanager.h>

#include <QFile>

namespace Test
{

/** Read the entire file content. */
inline QByteArray readFile(const QString &fn)
{
    QFile f(fn);
    f.open(QFile::ReadOnly);
    return f.readAll();
}

/** Delete all reservations. */
inline void clearAll(ReservationManager *mgr)
{
    const auto batches = mgr->batches(); // copy, as this is getting modified in the process
    for (const auto &id : batches) {
        mgr->removeBatch(id);
    }
    Q_ASSERT(mgr->batches().empty());
}

/** Delete all passes. */
inline void clearAll(PkPassManager *mgr)
{
    for (const auto &id : mgr->passes()) {
        mgr->removePass(id);
    }
    Q_ASSERT(mgr->passes().isEmpty());
}

/** Delete all documents. */
inline void clearAll(DocumentManager *docMgr)
{
    for (const auto &id : docMgr->documents()) {
        docMgr->removeDocument(id);
    }
    Q_ASSERT(docMgr->documents().isEmpty());
}

}

#endif
