/*
    Copyright (C) 2019 Nicolas Fella <nicolas.fella@gmx.de>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LOCKMANAGER_H
#define LOCKMANAGER_H

#include <QObject>

class LockBackend : public QObject
{
public:
    explicit LockBackend(QObject *parent = nullptr) : QObject(parent)
    {}
    virtual ~LockBackend() = default;
    virtual void setInhibitionOn() = 0;
    virtual void setInhibitionOff() = 0;
};

class LockManager : public QObject
{
    Q_OBJECT

public:
    explicit LockManager(QObject *parent = nullptr);
    ~LockManager();

public Q_SLOTS:
    void toggleInhibitScreenLock();
private:
    LockBackend *m_backend;
    bool m_inhibit;
};

#endif // LOCKMANAGER_H


