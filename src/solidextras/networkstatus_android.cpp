/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "networkstatus.h"

#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>

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

    QJniObject m_obj;
};

static void networkStatusChangedCallback()
{
    Q_EMIT NetworkStatusBackend::instance()->networkStatusChanged();
}

NetworkStatusBackend::NetworkStatusBackend(QObject *parent)
    : QObject(parent)
{
    QJniEnvironment env;
    jclass cls = env->FindClass("org/kde/solidextras/NetworkStatus");
    static JNINativeMethod methods = {"networkStatusChanged", "()V", reinterpret_cast<void *>(networkStatusChangedCallback)};
    env->RegisterNatives(cls, &methods, sizeof(methods) / sizeof(JNINativeMethod));

#if QT_VERSION <QT_VERSION_CHECK(6, 7, 0)
    m_obj = QJniObject("org/kde/solidextras/NetworkStatus", "(Landroid/content/Context;)V", QNativeInterface::QAndroidApplication::context());
#else
    m_obj = QJniObject("org/kde/solidextras/NetworkStatus", QNativeInterface::QAndroidApplication::context());
#endif
}

NetworkStatusBackend* NetworkStatusBackend::instance()
{
    static NetworkStatusBackend s_instance;
    return &s_instance;
}

bool NetworkStatusBackend::connectivity() const
{
#if QT_VERSION <QT_VERSION_CHECK(6, 7, 0)
    return m_obj.callMethod<jboolean>("connectivity", "(Landroid/content/Context;)Z", QNativeInterface::QAndroidApplication::context());
#else
    return m_obj.callMethod<jboolean>("connectivity", QNativeInterface::QAndroidApplication::context());
#endif
}

bool NetworkStatusBackend::metered() const
{
#if QT_VERSION <QT_VERSION_CHECK(6, 7, 0)
    return m_obj.callMethod<jboolean>("metered", "(Landroid/content/Context;)Z", QNativeInterface::QAndroidApplication::context());
#else
    return m_obj.callMethod<jboolean>("metered", QNativeInterface::QAndroidApplication::context());
#endif
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
