/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SOLIDLOCKBACKEND_H
#define SOLIDLOCKBACKEND_H

#include <QObject>
#include "lockmanager.h"

class OrgFreedesktopScreenSaverInterface;

class SolidLockBackend : public LockBackend
{

public:
    explicit SolidLockBackend(QObject *parent = nullptr);

    void setInhibitionOff() override;
    void setInhibitionOn(const QString &explanation) override;

private:
    OrgFreedesktopScreenSaverInterface* m_iface;
    int m_cookie;
};

#endif //  SOLIDLOCKBACKEND_H

