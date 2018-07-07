/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pkpassmanager.h"
#include "logging.h"

#include <KItinerary/Reservation>
#include <KPkPass/Pass>

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

    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes/") + passId + QLatin1String(".pkpass");
    return QFile::exists(passPath);
}

KPkPass::Pass* PkPassManager::pass(const QString& passId)
{
    const auto it = m_passes.constFind(passId);
    if (it != m_passes.constEnd() && it.value()) {
        return it.value();
    }

    const QString passPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes/") + passId + QLatin1String(".pkpass");
    if (!QFile::exists(passPath)) {
        return nullptr;
    }

    auto file = KPkPass::Pass::fromFile(passPath, this);
    // TODO error handling
    m_passes.insert(passId, file);
    return file;
}

QObject* PkPassManager::passObject(const QString& passId)
{
    return pass(passId);
}

QString PkPassManager::passId(const QVariant &reservation) const
{
    if (!JsonLd::canConvert<Reservation>(reservation)) {
        return {};
    }
    const auto res = JsonLd::convert<Reservation>(reservation);
    const auto passTypeId = res.pkpassPassTypeIdentifier();
    const auto serialNum = res.pkpassSerialNumber();
    if (passTypeId.isEmpty() || serialNum.isEmpty()) {
        return {};
    }
    return passTypeId + QLatin1Char('/') + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding));
}

void PkPassManager::importPass(const QUrl& url)
{
    doImportPass(url, Copy);
}

void PkPassManager::importPassFromTempFile(const QUrl& tmpFile)
{
    doImportPass(tmpFile, Move);
}

void PkPassManager::doImportPass(const QUrl& url, PkPassManager::ImportMode mode)
{
    qCDebug(Log) << url << mode;
    if (!url.isLocalFile())
        return; // TODO

    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes");
    QDir::root().mkpath(basePath);

    std::unique_ptr<KPkPass::Pass> newPass(KPkPass::Pass::fromFile(url.toLocalFile()));
    if (!newPass)
        return; // TODO error handling
    if (newPass->passTypeIdentifier().isEmpty() || newPass->serialNumber().isEmpty())
        return; // TODO error handling

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
            QFile::rename(url.toLocalFile(), dir.absoluteFilePath(serNum + QLatin1String(".pkpass")));
            break;
        case Copy:
            QFile::copy(url.toLocalFile(), dir.absoluteFilePath(serNum + QLatin1String(".pkpass")));
            break;
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
        emit passUpdated(passId, changes);
        oldPass->deleteLater();
    } else {
        emit passAdded(passId);
    }
}

void PkPassManager::removePass(const QString& passId)
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes/");
    QFile::remove(basePath + QLatin1Char('/') + passId + QLatin1String(".pkpass"));
    emit passRemoved(passId);
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

void PkPassManager::updatePasses()
{
    for (const auto &passId : passes())
        updatePass(passId);
}

QDateTime PkPassManager::relevantDate(KPkPass::Pass *pass)
{
    const auto dt = pass->relevantDate();
    if (dt.isValid())
        return dt;
    return pass->expirationDate();
}
