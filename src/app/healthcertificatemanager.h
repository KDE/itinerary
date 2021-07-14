/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HEALTHCERTIFICATEMANAGER_H
#define HEALTHCERTIFICATEMANAGER_H

#include <QAbstractListModel>

#include <vector>

class QByteArray;

/** Model/manager for digital vaccination certificates. */
class HealthCertificateManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isAvailable READ isAvailable CONSTANT)
public:
    explicit HealthCertificateManager(QObject *parent = nullptr);
    ~HealthCertificateManager();

    /** Health certificate support is compiled in. */
    bool isAvailable() const;

    void importCertificate(const QByteArray &rawData);
    Q_INVOKABLE void importCertificateFromClipboard();
    Q_INVOKABLE void removeCertificate(int row);

    enum {
        CertificateRole = Qt::UserRole,
        RawDataRole,
        StorageIdRole,
    };

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct CertData {
        QString name;
        QVariant cert;
    };
    static bool certLessThan(const CertData &lhs, const CertData &rhs);

    void loadCertificates();
    QByteArray certificateRawData(const CertData &certData) const;

    std::vector<CertData> m_certificates;
};

#endif // HEALTHCERTIFICATEMANAGER_H
