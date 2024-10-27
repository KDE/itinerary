/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"

#include "healthcertificatemanager.h"

#if HAVE_KHEALTHCERTIFICATE
#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KRecoveryCertificate>
#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>
#endif

#include <KLocalizedString>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>

HealthCertificateManager::HealthCertificateManager(QObject *parent)
    : QAbstractListModel(parent)
{
    loadCertificates();
}

HealthCertificateManager::~HealthCertificateManager() = default;

bool HealthCertificateManager::isAvailable()
{
#if HAVE_KHEALTHCERTIFICATE
    return true;
#else
    return false;
#endif
}

static QString basePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/health-certificates/");
}

bool HealthCertificateManager::importCertificate(const QByteArray &rawData)
{
    // check whether we know this certificate already
    for (const auto &c : m_certificates) {
        if (certificateRawData(c.cert) == rawData) {
            return true;
        }
    }
#if HAVE_KHEALTHCERTIFICATE
    CertData certData;
    certData.cert = KHealthCertificateParser::parse(rawData);
    if (certData.cert.isNull()) {
        return false;
    }

    auto path = basePath();
    QDir().mkpath(path);
    certData.name = QUuid::createUuid().toString(QUuid::WithoutBraces);
    path += QLatin1Char('/') + certData.name;

    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << f.errorString() << f.fileName();
        return false;
    }
    f.write(rawData);
    f.close();

    const auto it = std::lower_bound(m_certificates.begin(), m_certificates.end(), certData, certLessThan);
    const auto row = std::distance(m_certificates.begin(), it);
    beginInsertRows({}, row, row);
    m_certificates.insert(it, std::move(certData));
    endInsertRows();
    Q_EMIT newCertificateLoaded(row);
    return true;
#else
    return false;
#endif
}

void HealthCertificateManager::removeCertificate(int row)
{
    beginRemoveRows({}, row, row);
    const auto it = m_certificates.begin() + row;
    QFile::remove(basePath() + QLatin1Char('/') + (*it).name);
    m_certificates.erase(it);
    endRemoveRows();
}

int HealthCertificateManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_certificates.size();
}

QVariant HealthCertificateManager::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index) || index.row() < 0) {
        return {};
    }

    const auto &v = m_certificates[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return displayName(v.cert);
    case CertificateRole:
        return v.cert;
    case RawDataRole:
        return certificateRawData(v.cert);
    case StorageIdRole:
        return v.name;
    }
    return {};
}

QHash<int, QByteArray> HealthCertificateManager::roleNames() const
{
    auto rns = QAbstractListModel::roleNames();
    rns.insert(CertificateRole, "certificate");
    rns.insert(RawDataRole, "rawData");
    rns.insert(StorageIdRole, "storageId");
    return rns;
}

QString HealthCertificateManager::displayName(const QVariant &certificate)
{
#if HAVE_KHEALTHCERTIFICATE
    if (certificate.userType() == qMetaTypeId<KVaccinationCertificate>()) {
        const auto cert = certificate.value<KVaccinationCertificate>();
        if (cert.dose() > 0 && cert.totalDoses() > 0) {
            return i18n("Vaccination %1/%2 (%3)", cert.dose(), cert.totalDoses(), cert.name());
        }
        return i18n("Vaccination (%1)", cert.name());
    }
    if (certificate.userType() == qMetaTypeId<KTestCertificate>()) {
        const auto cert = certificate.value<KTestCertificate>();
        return i18n("Test %1 (%2)",
                    QLocale().toString(cert.date().isValid() ? cert.date() : cert.certificateIssueDate().date(), QLocale::NarrowFormat),
                    cert.name());
    }
    if (certificate.userType() == qMetaTypeId<KRecoveryCertificate>()) {
        const auto cert = certificate.value<KRecoveryCertificate>();
        return i18n("Recovery (%1)", cert.name());
    }
#endif
    return {};
}

void HealthCertificateManager::loadCertificates()
{
    beginResetModel();
    for (QDirIterator it(basePath(), QDir::Files); it.hasNext();) {
        QFile f(it.next());
        if (!f.open(QFile::ReadOnly)) {
            qWarning() << f.errorString() << f.fileName();
            continue;
        }

        const auto rawData = f.readAll();
        CertData certData;
        certData.name = it.fileName();
#if HAVE_KHEALTHCERTIFICATE
        certData.cert = KHealthCertificateParser::parse(rawData);
#endif
        if (certData.cert.isNull()) {
            continue;
        }

        m_certificates.push_back(std::move(certData));
    }
    std::sort(m_certificates.begin(), m_certificates.end(), certLessThan);
    endResetModel();
}

QByteArray HealthCertificateManager::certificateRawData([[maybe_unused]] const QVariant &cert)
{
#if HAVE_KHEALTHCERTIFICATE
    if (cert.userType() == qMetaTypeId<KVaccinationCertificate>()) {
        return cert.value<KVaccinationCertificate>().rawData();
    }
    if (cert.userType() == qMetaTypeId<KTestCertificate>()) {
        return cert.value<KTestCertificate>().rawData();
    }
    if (cert.userType() == qMetaTypeId<KRecoveryCertificate>()) {
        return cert.value<KRecoveryCertificate>().rawData();
    }
#endif
    return {};
}

bool HealthCertificateManager::certLessThan(const CertData &lhs, const CertData &rhs)
{
#if HAVE_KHEALTHCERTIFICATE
    const auto lhsDt = KHealthCertificate::relevantUntil(lhs.cert);
    const auto rhsDt = KHealthCertificate::relevantUntil(rhs.cert);
    if (lhsDt == rhsDt) {
        return lhs.name < rhs.name;
    }
    if (!lhsDt.isValid()) {
        return false;
    }
    return !rhsDt.isValid() || lhsDt > rhsDt;
#else
    return lhs.name < rhs.name;
#endif
}

#include "moc_healthcertificatemanager.cpp"
