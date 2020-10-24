/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "networkstatus.h"

#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

using namespace SolidExtras;

class NetworkStatusBackend : public QObject
{
    Q_OBJECT
public:
    static NetworkStatusBackend* instance();
    bool connectivity() const;
    bool metered() const;

Q_SIGNALS:
    void networkStatusChanged();

private:
    explicit NetworkStatusBackend(QObject *parent = nullptr);

    QAndroidJniObject m_obj;
};

static void networkStatusChangedCallback()
{
    Q_EMIT NetworkStatusBackend::instance()->networkStatusChanged();
}

NetworkStatusBackend::NetworkStatusBackend(QObject *parent)
    : QObject(parent)
{
    QAndroidJniEnvironment env;
    jclass cls = env->FindClass("org/kde/solidextras/NetworkStatus");
    static JNINativeMethod methods = {"networkStatusChanged", "()V", reinterpret_cast<void *>(networkStatusChangedCallback)};
    env->RegisterNatives(cls, &methods, sizeof(methods) / sizeof(JNINativeMethod));

    m_obj = QAndroidJniObject("org/kde/solidextras/NetworkStatus", "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
}

NetworkStatusBackend* NetworkStatusBackend::instance()
{
    static NetworkStatusBackend s_instance;
    return &s_instance;
}

bool NetworkStatusBackend::connectivity() const
{
    return m_obj.callMethod<jboolean>("connectivity", "(Landroid/content/Context;)Z", QtAndroid::androidContext().object());
}

bool NetworkStatusBackend::metered() const
{
    return m_obj.callMethod<jboolean>("metered", "(Landroid/content/Context;)Z", QtAndroid::androidContext().object());
}


NetworkStatus::NetworkStatus(QObject *parent)
    : QObject(parent)
{
    connect(NetworkStatusBackend::instance(), &NetworkStatusBackend::networkStatusChanged, this, &NetworkStatus::connectivityChanged);
    connect(NetworkStatusBackend::instance(), &NetworkStatusBackend::networkStatusChanged, this, &NetworkStatus::meteredChanged);
}

NetworkStatus::State NetworkStatus::connectivity() const
{
    return NetworkStatusBackend::instance()->connectivity() ? NetworkStatus::Yes : NetworkStatus::No;
}

NetworkStatus::State NetworkStatus::metered() const
{
    return NetworkStatusBackend::instance()->metered() ? NetworkStatus::Yes : NetworkStatus::No;
}

#include "networkstatus_android.moc"
