/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"

#include "kdeconnect.h"

#if HAVE_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#endif

#include <QDebug>
#include <QList>
#include <QUrl>

KDEConnectDeviceModel::KDEConnectDeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

KDEConnectDeviceModel::~KDEConnectDeviceModel() = default;

void KDEConnectDeviceModel::refresh()
{
    if (!m_devices.empty()) // ### this is wrong, but the consumer code can't handle proper update yet...
        return;

#if HAVE_DBUS
    beginResetModel();

    // TODO we might want to do all this asynchronously by watching change signals and cache the device list
    auto msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kdeconnect"),
                                              QStringLiteral("/modules/kdeconnect"),
                                              QStringLiteral("org.kde.kdeconnect.daemon"),
                                              QStringLiteral("devices"));
    msg.setArguments({true, true});
    QDBusPendingReply<QStringList> reply = QDBusConnection::sessionBus().asyncCall(msg);
    reply.waitForFinished();

    if (!reply.isValid()) {
        return;
    }

    const auto values = reply.value();
    for (const QString &deviceId : values) {
        QDBusInterface deviceIface(QStringLiteral("org.kde.kdeconnect"),
                                   QStringLiteral("/modules/kdeconnect/devices/") + deviceId,
                                   QStringLiteral("org.kde.kdeconnect.device"));
        QDBusReply<bool> pluginReply = deviceIface.call(QStringLiteral("hasPlugin"), QLatin1StringView("kdeconnect_share"));

        if (pluginReply.value()) {
            m_devices.push_back({deviceId, deviceIface.property("name").toString()});
        }
    }
    endResetModel();
#endif
}

int KDEConnectDeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_devices.size();
}

QVariant KDEConnectDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case DeviceNameRole:
        return m_devices[index.row()].name;
    case DeviceIdRole:
        return m_devices[index.row()].deviceId;
    }

    return {};
}

QHash<int, QByteArray> KDEConnectDeviceModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(DeviceNameRole, "name");
    r.insert(DeviceIdRole, "deviceId");
    return r;
}

void KDEConnect::sendToDevice(const QString &fileName, const QString &deviceId)
{
#if HAVE_DBUS
    const QString method = QStringLiteral("openFile");

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kdeconnect"),
                                                      QStringLiteral("/modules/kdeconnect/devices/") + deviceId + QStringLiteral("/share"),
                                                      QStringLiteral("org.kde.kdeconnect.device.share"),
                                                      method);
    msg.setArguments({QUrl::fromLocalFile(fileName).toString()});

    QDBusConnection::sessionBus().asyncCall(msg);
#endif
}

#include "moc_kdeconnect.cpp"
