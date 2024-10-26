/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BRIGHTNESSMANAGER_H
#define BRIGHTNESSMANAGER_H

#include "solidextras_export.h"

#include <QObject>

class BrightnessBackend : public QObject
{
public:
    explicit BrightnessBackend(QObject *parent = nullptr)
        : QObject(parent)
        , m_maximized()
        , m_previousValue()
    {
    }
    ~BrightnessBackend() override = default;

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

class SOLIDEXTRAS_EXPORT BrightnessManager : public QObject
{
    Q_OBJECT

public:
    explicit BrightnessManager(QObject *parent = nullptr);
    ~BrightnessManager() override;

public Q_SLOTS:
    void toggleBrightness();

private:
    BrightnessBackend *m_backend;
};

#endif //  BRIGHTNESSMANAGER_H
