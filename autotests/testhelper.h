/*
    SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include "applicationcontroller.h"
#include "documentmanager.h"
#include "passmanager.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"

#include <QAbstractItemModel>
#include <QFile>
#include <QJsonDocument>
#include <QProcess>
#include <QSignalSpy>

namespace Test
{

template <typename T>
inline bool compareJson(const QString &refFile, const T &output, const T &ref)
{
    if (output != ref) {
        QFile failFile(refFile + QLatin1String(".fail"));
        failFile.open(QFile::WriteOnly);
        failFile.write(QJsonDocument(output).toJson());
        failFile.close();

        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), refFile, failFile.fileName()});
        if (!proc.waitForFinished()) {
            // e.g. Windows CI not having diff
            qDebug() << "Actual:";
            qDebug().noquote() << output;
            qDebug() << "Expected:";
            qDebug().noquote() << ref;
        }
        return false;
    }
    return true;
}


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

inline void clearAll(PassManager *passMgr)
{
    while (passMgr->rowCount()) {
        passMgr->removeRow(0);
    }
    Q_ASSERT(passMgr->rowCount() == 0);
}

/** Fully set up application controller. */
inline std::unique_ptr<ApplicationController> makeAppController()
{
    std::unique_ptr<ApplicationController> ctrl(new ApplicationController);

    auto passMgr = new PassManager(ctrl.get());
    clearAll(passMgr);
    ctrl->setPassManager(passMgr);
    return ctrl;
}

/** Wait for a model reset before proceeding. */
inline void waitForReset(QAbstractItemModel *model)
{
    QSignalSpy resetSpy(model, &QAbstractItemModel::modelReset);
    resetSpy.wait(10);
}

}

#endif
