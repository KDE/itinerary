/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SOLIDBRIGHTNESSBACKEND_H
#define SOLIDBRIGHTNESSBACKEND_H

#include <QObject>
#include "brightnessmanager.h"

class OrgKdeSolidPowerManagementActionsBrightnessControlInterface;

class SolidBrightnessBackend : public BrightnessBackend
{

public:
    explicit SolidBrightnessBackend(QObject *parent = nullptr);
    ~SolidBrightnessBackend() override;

    void setBrightness(float brightness) override;
    float brightness() const override;
    float maxBrightness() const override;

private:
    OrgKdeSolidPowerManagementActionsBrightnessControlInterface *m_iface;
};

#endif //  SOLIDBRIGHTNESSBACKEND_H
