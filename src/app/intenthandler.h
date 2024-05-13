/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef INTENTHANDLER_H
#define INTENTHANDLER_H

#include <QObject>

class IntentHandlerPrivate;

namespace KAndroidExtras {
class Intent;
}

/** Provides access to incoming Android Intents. */
class IntentHandler : public QObject
{
    Q_OBJECT
public:
    explicit IntentHandler(QObject *parent = nullptr);
    ~IntentHandler() override;

Q_SIGNALS:
    /** Emitted when a new Intent is received in Android' Activity::onNewIntent. */
    void handleIntent(const KAndroidExtras::Intent &intent);

private:
    friend class IntentHandlerPrivate;
    void onNewIntent(const KAndroidExtras::Intent &intent);
    static IntentHandler *s_instance;
};

#endif // APPLICATIONCONTROLLER_H
