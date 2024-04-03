/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pkpassmanager.h"
#include "genericpkpass.h"
#include "logging.h"

#include <KItinerary/DocumentUtil>
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

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

PkPassManager::PkPassManager(QObject* parent)
    : QObject(parent)
{
}

PkPassManager::~PkPassManager() = default;

void PkPassManager::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory)
{
    m_namFactory = namFactory;
}

QVector<QString> PkPassManager::passes()
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes"_L1;
    QDir::root().mkpath(basePath);

    QVector<QString> passIds;
    for (QDirIterator topIt(basePath, QDir::NoDotAndDotDot | QDir::Dirs); topIt.hasNext();) {
        for (QDirIterator subIt(topIt.next(), QDir::Files); subIt.hasNext();) {
            QFileInfo fi(subIt.next());
            passIds.push_back(fi.dir().dirName() + '/'_L1 + fi.baseName());
        }
    }

    return passIds;
}

bool PkPassManager::hasPass(const QString &passId) const
{
    if (m_passes.contains(passId)) {
        return true;
    }

    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes/"_L1 + passId + ".pkpass"_L1;
    return QFile::exists(passPath);
}

KPkPass::Pass* PkPassManager::pass(const QString& passId)
{
    const auto it = m_passes.constFind(passId);
    if (it != m_passes.constEnd() && it.value()) {
        return it.value();
    }

    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes/"_L1 + passId + ".pkpass"_L1;
    if (!QFile::exists(passPath)) {
        return nullptr;
    }

    auto file = KPkPass::Pass::fromFile(passPath, this);
    // TODO error handling
    m_passes.insert(passId, file);
    return file;
}

QString PkPassManager::passId(const QVariant &reservation)
{
    QString passTypeId, serialNum;

    const auto pkPassUrl = DocumentUtil::pkPassId(reservation);
    if (pkPassUrl.isValid()) {
        passTypeId = pkPassUrl.host();
        serialNum = pkPassUrl.path().mid(1);
    } else if (JsonLd::canConvert<Reservation>(reservation)) {
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
    return passTypeId + '/'_L1 + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding));
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

    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes"_L1;
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
    QString passId = dir.dirName() + '/'_L1 + serNum;

    auto oldPass = pass(passId);
    if (oldPass) {
        QFile::remove(dir.absoluteFilePath(serNum + ".pkpass"_L1));
        m_passes.remove(passId);
    }

    switch (mode) {
        case Move:
            QFile::rename(fileName, dir.absoluteFilePath(serNum + ".pkpass"_L1));
            break;
        case Copy:
            QFile::copy(fileName, dir.absoluteFilePath(serNum + ".pkpass"_L1));
            break;
        case Data:
        {
            QFile f(dir.absoluteFilePath(serNum + ".pkpass"_L1));
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
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes/"_L1;
    QFile::remove(basePath + '/'_L1 + passId + ".pkpass"_L1);
    Q_EMIT passRemoved(passId);
    delete m_passes.take(passId);
}

void PkPassManager::updatePass(const QString& passId)
{
    auto p = pass(passId);
    if (!canUpdate(p)) {
        return;
    }

    QNetworkRequest req(p->passUpdateUrl());
    req.setRawHeader("Authorization", "ApplePass " + p->authenticationToken().toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    qDebug() << req.url();
    auto reply = m_namFactory()->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        qDebug() << reply->errorString();
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

QDateTime PkPassManager::updateTime(const QString &passId)
{
    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes/"_L1 + passId + ".pkpass"_L1;
    QFileInfo fi(passPath);
    return fi.lastModified();
}

QDateTime PkPassManager::relevantDate(KPkPass::Pass *pass)
{
    auto dt = pass->relevantDate();
    if (dt.isValid()) {
        return dt;
    }
    return pass->expirationDate();
}

bool PkPassManager::canUpdate(KPkPass::Pass *pass)
{
    return pass && pass->webServiceUrl().isValid() && !pass->authenticationToken().isEmpty();
}

QByteArray PkPassManager::rawData(const QString &passId)
{
    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/passes/"_L1 + passId + ".pkpass"_L1;
    QFile f(passPath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open pass file for pass" << passId;
        return {};
    }
    return f.readAll();
}

#include "moc_pkpassmanager.cpp"
