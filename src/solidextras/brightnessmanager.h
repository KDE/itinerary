/*
    Copyright (C) 2018 Nicolas Fella <nicolas.fella@gmx.de>

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

#ifndef BRIGHTNESSMANAGER_H
#define BRIGHTNESSMANAGER_H

#include <QObject>

class BrightnessBackend : public QObject
{
public:
    explicit BrightnessBackend(QObject *parent = nullptr) : QObject(parent)
    , m_maximized()
    , m_previousValue()
    {}
    virtual ~BrightnessBackend() = default;

public:
    virtual void toggleBrightness();

protected:
    virtual void setBrightness(float brightness) = 0;
    virtual float brightness() const = 0;
    virtual float maxBrightness() const = 0;

private:
    bool m_maximized;
    int m_previousValue;
};

class BrightnessManager : public QObject
{
    Q_OBJECT

public:
    explicit BrightnessManager(QObject *parent = nullptr);
    ~BrightnessManager();

public Q_SLOTS:
    void toggleBrightness();

private:
    BrightnessBackend *m_backend;
};

#endif //  BRIGHTNESSMANAGER_H

