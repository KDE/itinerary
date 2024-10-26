/*
   SPDX-FileCopyrightText: 2017-2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDECONNECT_H
#define KDECONNECT_H

#include <QAbstractListModel>
#include <QObject>
#include <QString>

class KDEConnectDeviceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit KDEConnectDeviceModel(QObject *parent = nullptr);
    ~KDEConnectDeviceModel();

    Q_INVOKABLE void refresh();

    enum {
        DeviceNameRole = Qt::DisplayRole,
        DeviceIdRole = Qt::UserRole,
    };

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Device {
        QString deviceId;
        QString name;
    };
    std::vector<Device> m_devices;
};

class KDEConnect : public QObject
{
    Q_OBJECT
public:
    static void sendToDevice(const QString &fileName, const QString &deviceId);
};

#endif
