/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ANDROIDLOCKBACKEND_H
#define ANDROIDLOCKBACKEND_H

#include <QObject>
#include "lockmanager.h"


class AndroidLockBackend : public LockBackend
{

public:
    explicit AndroidLockBackend(QObject *parent = nullptr);
    virtual ~AndroidLockBackend();

    void setInhibitionOff() override;
    void setInhibitionOn(const QString &explanation) override;
};

#endif //  ANDROIDBRIGHTNESSBACKEND_H

