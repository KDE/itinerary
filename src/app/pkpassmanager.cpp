/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassmanager.h"
#include "genericpkpass.h"
#include "logging.h"

#include <KItinerary/Reservation>
#include <KPkPass/Pass>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUrl>
#include <QVector>

using namespace KItinerary;

PkPassManager::PkPassManager(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

PkPassManager::~PkPassManager() = default;

QVector<QString> PkPassManager::passes() const
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes");
    QDir::root().mkpath(basePath);

    QVector<QString> passIds;
    for (QDirIterator topIt(basePath, QDir::NoDotAndDotDot | QDir::Dirs); topIt.hasNext();) {
        for (QDirIterator subIt(topIt.next(), QDir::Files); subIt.hasNext();) {
            QFileInfo fi(subIt.next());
            passIds.push_back(fi.dir().dirName() + QLatin1Char('/') + fi.baseName());
        }
    }

    return passIds;
}

bool PkPassManager::hasPass(const QString &passId) const
{
    if (m_passes.contains(passId)) {
        return true;
    }

    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/passes/") + passId + QLatin1String(".pkpass");
    return QFile::exists(passPath);
}

KPkPass::Pass* PkPassManager::pass(const QString& passId)
{
    const auto it = m_passes.constFind(passId);
    if (it != m_passes.constEnd() && it.value()) {
        return it.value();
    }

    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/passes/") + passId + QLatin1String(".pkpass");
    if (!QFile::exists(passPath)) {
        return nullptr;
    }

    auto file = KPkPass::Pass::fromFile(passPath, this);
    // TODO error handling
    m_passes.insert(passId, file);
    return file;
}

QString PkPassManager::passId(const QVariant &reservation) const
{
    QString passTypeId, serialNum;

    if (JsonLd::canConvert<Reservation>(reservation)) {
        const auto res = JsonLd::convert<Reservation>(reservation);
        passTypeId = res.pkpassPassTypeIdentifier();
        serialNum = res.pkpassSerialNumber();
    } else if (JsonLd::isA<GenericPkPass>(reservation)) {
        const auto p = JsonLd::convert<GenericPkPass>(reservation);
        passTypeId = p.pkpassPassTypeIdentifier();
        serialNum = p.pkpassSerialNumber();
    }

    if (passTypeId.isEmpty() || serialNum.isEmpty()) {
        return {};
    }
    return passTypeId + QLatin1Char('/') + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding));
}

QString PkPassManager::importPass(const QUrl& url)
{
    return doImportPass(url, {}, Copy);
}

void PkPassManager::importPassFromTempFile(const QUrl& tmpFile)
{
    doImportPass(tmpFile, {}, Move);
}

QString PkPassManager::importPassFromData(const QByteArray &data)
{
    return doImportPass({}, data, Data);
}

QString PkPassManager::doImportPass(const QUrl& url, const QByteArray &data, PkPassManager::ImportMode mode)
{
    qCDebug(Log) << url << mode;
    if (url.isEmpty() && data.isEmpty()) {
        return {};
    }

    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes");
    const auto fileName = url.isLocalFile() ? url.toLocalFile() : url.toString();
    QDir::root().mkpath(basePath);

    std::unique_ptr<KPkPass::Pass> newPass;
    if (!url.isEmpty()) {
        newPass.reset(KPkPass::Pass::fromFile(fileName));
    } else {
        newPass.reset(KPkPass::Pass::fromData(data));
    }

    if (!newPass) {
        qCDebug(Log) << "Failed to load pkpass file" << url;
        return {};
    }
    if (newPass->passTypeIdentifier().isEmpty() || newPass->serialNumber().isEmpty()) {
        qCDebug(Log) << "PkPass file has no type identifier or serial number" << url;
        return {};
    }

    QDir dir(basePath);
    dir.mkdir(newPass->passTypeIdentifier());
    dir.cd(newPass->passTypeIdentifier());

    // serialNumber() can contain percent-encoding or slashes,
    // ie stuff we don't want to have in file names
    const auto serNum = QString::fromUtf8(newPass->serialNumber().toUtf8().toBase64(QByteArray::Base64UrlEncoding));
    const QString passId = dir.dirName() + QLatin1Char('/') + serNum;

    auto oldPass = pass(passId);
    if (oldPass) {
        QFile::remove(dir.absoluteFilePath(serNum + QLatin1String(".pkpass")));
        m_passes.remove(passId);
    }

    switch (mode) {
        case Move:
            QFile::rename(fileName, dir.absoluteFilePath(serNum + QLatin1String(".pkpass")));
            break;
        case Copy:
            QFile::copy(fileName, dir.absoluteFilePath(serNum + QLatin1String(".pkpass")));
            break;
        case Data:
        {
            QFile f(dir.absoluteFilePath(serNum + QLatin1String(".pkpass")));
            if (!f.open(QFile::WriteOnly)) {
                qCWarning(Log) << "Failed to open file" << f.fileName() << f.errorString();
                break;
            }
            f.write(data);
        }
    }

    if (oldPass) {
        // check for changes and generate change message
        QStringList changes;
        for (const auto &f : newPass->fields()) {
            const auto prevValue = oldPass->field(f.key()).value();
            const auto curValue = f.value();
            if (curValue != prevValue) {
                changes.push_back(f.changeMessage());
            }
        }
        Q_EMIT passUpdated(passId, changes);
        oldPass->deleteLater();
    } else {
        Q_EMIT passAdded(passId);
    }

    return passId;
}

void PkPassManager::removePass(const QString& passId)
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes/");
    QFile::remove(basePath + QLatin1Char('/') + passId + QLatin1String(".pkpass"));
    Q_EMIT passRemoved(passId);
    delete m_passes.take(passId);
}

void PkPassManager::updatePass(const QString& passId)
{
    auto p = pass(passId);
    if (!p || p->webServiceUrl().isEmpty() || p->authenticationToken().isEmpty())
        return;
    if (relevantDate(p) < QDateTime::currentDateTimeUtc()) // TODO check expiration date and voided property
        return;

    QNetworkRequest req(p->passUpdateUrl());
    req.setRawHeader("Authorization", "ApplePass " + p->authenticationToken().toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(Log) << "Failed to download pass:" << reply->errorString();
            return;
        }

        QTemporaryFile tmp;
        tmp.open();
        tmp.write(reply->readAll());
        tmp.close();
        importPassFromTempFile(QUrl::fromLocalFile(tmp.fileName()));
    });
}

QDateTime PkPassManager::updateTime(const QString &passId) const
{
    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/passes/") + passId + QLatin1String(".pkpass");
    QFileInfo fi(passPath);
    return fi.lastModified();
}

QDateTime PkPassManager::relevantDate(KPkPass::Pass *pass)
{
    const auto dt = pass->relevantDate();
    if (dt.isValid())
        return dt;
    return pass->expirationDate();
}

QByteArray PkPassManager::rawData(const QString &passId) const
{
    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/passes/") + passId + QLatin1String(".pkpass");
    QFile f(passPath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open pass file for pass" << passId;
        return {};
    }
    return f.readAll();
}

#include "moc_pkpassmanager.cpp"
