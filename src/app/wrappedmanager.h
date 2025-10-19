// SPDX-CopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <qqmlregistration.h>

class WrappedManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
public:
    explicit WrappedManager(QObject *parent = nullptr);

    [[nodiscard]] bool available() const;

Q_SIGNALS:
    void availableChanged();
};
