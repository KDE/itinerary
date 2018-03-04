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

#include <KPkPass/Pass>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QStandardPaths>
#include <QUrl>
#include <QVector>

PkPassManager::PkPassManager(QObject* parent)
    : QObject(parent)
{
}

PkPassManager::~PkPassManager() = default;

QVector<QString> PkPassManager::passes() const
{
    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes");
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

KPkPass::Pass* PkPassManager::pass(const QString& passId)
{
    const auto it = m_passes.constFind(passId);
    if (it != m_passes.constEnd())
        return it.value();

    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes/");
    auto file = KPkPass::Pass::fromFile(basePath + passId + QLatin1String(".pkpass"), this);
    // TODO error handling
    m_passes.insert(passId, file);
    return file;
}

void PkPassManager::importPass(const QUrl& url)
{
    doImportPass(url, Copy);
}

void PkPassManager::importPassFromTempFile(const QString& tmpFile)
{
    doImportPass(QUrl::fromLocalFile(tmpFile), Move);
}

void PkPassManager::doImportPass(const QUrl& url, PkPassManager::ImportMode mode)
{
    qCDebug(Log) << url << mode;
    if (!url.isLocalFile())
        return; // TODO

    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes");
    QDir::root().mkpath(basePath);

    std::unique_ptr<KPkPass::Pass> file(KPkPass::Pass::fromFile(url.toLocalFile()));
    if (!file)
        return; // TODO error handling
    if (file->passTypeIdentifier().isEmpty() || file->serialNumber().isEmpty())
        return; // TODO error handling

    // TODO check for already existing version
    QDir dir(basePath);
    dir.mkdir(file->passTypeIdentifier());
    dir.cd(file->passTypeIdentifier());

    // serialNumber() can contain percent-encoding or slashes,
    // ie stuff we don't want to have in file names
    const auto passId = QString::fromUtf8(file->serialNumber().toUtf8().toBase64(QByteArray::Base64UrlEncoding));

    switch (mode) {
        case Move:
            QFile::rename(url.toLocalFile(), dir.absoluteFilePath(passId + QLatin1String(".pkpass")));
            break;
        case Copy:
            QFile::copy(url.toLocalFile(), dir.absoluteFilePath(passId + QLatin1String(".pkpass")));
            break;
    }

    emit passAdded(dir.dirName() + QLatin1Char('/') + passId);
}

void PkPassManager::removePass(const QString& passId)
{
    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/passes/");
    QFile::remove(basePath + QLatin1Char('/') + passId + QLatin1String(".pkpass"));
    // TODO change signal
}
