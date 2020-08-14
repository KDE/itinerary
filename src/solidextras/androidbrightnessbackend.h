/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ANDROIDBRIGHTNESSBACKEND_H
#define ANDROIDBRIGHTNESSBACKEND_H

#include <QObject>
#include "brightnessmanager.h"


class AndroidBrightnessBackend : public BrightnessBackend
{

public:
    explicit AndroidBrightnessBackend(QObject *parent = nullptr);
    virtual ~AndroidBrightnessBackend();

    void setBrightness(float brightness) override;
    float brightness() const override;
    float maxBrightness() const override;
};

#endif //  ANDROIDBRIGHTNESSBACKEND_H
