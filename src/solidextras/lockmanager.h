/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOCKMANAGER_H
#define LOCKMANAGER_H

#include "solidextras_export.h"

#include <QObject>

class LockBackend : public QObject
{
public:
    explicit LockBackend(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~LockBackend() override = default;
    virtual void setInhibitionOn(const QString &explanation) = 0;
    virtual void setInhibitionOff() = 0;
};

class SOLIDEXTRAS_EXPORT LockManager : public QObject
{
    Q_OBJECT

public:
    explicit LockManager(QObject *parent = nullptr);
    ~LockManager() override;

public Q_SLOTS:
    /** Toggle screen lock inhibition.
     *  @param explanation A human-readable explanation on why the screen lock is inhibited.
     *  (not used on all platforms).
     */
    void toggleInhibitScreenLock(const QString &explanation);

private:
    LockBackend *m_backend;
    bool m_inhibit;
};

#endif // LOCKMANAGER_H
