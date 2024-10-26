/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentmanager.h"

#include "jsonio.h"
#include "logging.h"

#include <KItinerary/CreativeWork>
#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QJsonObject>
#include <QStandardPaths>
#include <QVariant>

using namespace KItinerary;

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
{
}

DocumentManager::~DocumentManager() = default;

QList<QString> DocumentManager::documents() const
{
    QList<QString> docs;

    QDirIterator it(basePath(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        docs.push_back(it.fileName());
    }

    return docs;
}

bool DocumentManager::hasDocument(const QString &id) const
{
    return QDir(basePath() + id).exists();
}

QVariant DocumentManager::documentInfo(const QString &id) const
{
    const QString filePath = basePath() + id + QLatin1StringView("/meta.json");
    QFile f(filePath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to load document meta data" << filePath << f.errorString();
        return {};
    }
    return JsonLdDocument::fromJsonSingular(JsonIO::read(f.readAll()).toObject());
}

QString DocumentManager::documentFilePath(const QString &id) const
{
    const auto info = documentInfo(id);
    const auto fileName = JsonLdDocument::readProperty(info, "name").toString();
    return basePath() + id + QLatin1Char('/') + fileName;
}

void DocumentManager::addDocument(const QString &id, const QVariant &info, const QByteArray &data)
{
    if (!JsonLd::canConvert<CreativeWork>(info)) {
        qCWarning(Log) << "Invalid document meta data" << info;
        return;
    }
    if (id.isEmpty()) {
        qCWarning(Log) << "Trying to add a document with an empty identifier!";
        return;
    }

    const QString path = basePath() + id + QLatin1Char('/');
    QDir().mkpath(path);

    const auto fileName = File::normalizeDocumentFileName(JsonLdDocument::readProperty(info, "name").toString());
    auto normalizedDocInfo = info;
    JsonLdDocument::writeProperty(normalizedDocInfo, "name", fileName);

    QFile dataFile(path + fileName);
    if (!dataFile.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store document to" << path << fileName << dataFile.errorString();
        // TODO error message for the ui
        return;
    }
    dataFile.write(data);
    dataFile.close();

    QFile metaFile(path + QLatin1StringView("meta.json"));
    if (!metaFile.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store document meta data" << path << metaFile.errorString();
        // TODO error message for the ui
        return;
    }
    metaFile.write(JsonIO::write(JsonLdDocument::toJson(normalizedDocInfo)));
    metaFile.close();
    Q_EMIT documentAdded(id);
}

void DocumentManager::addDocument(const QString &id, const QVariant &info, const QString &filePath)
{
    if (!JsonLd::canConvert<CreativeWork>(info)) {
        qCWarning(Log) << "Invalid document meta data" << info;
        return;
    }
    if (id.isEmpty()) {
        qCWarning(Log) << "Trying to add a document with an empty identifier!";
        return;
    }

    const QString path = basePath() + id + QLatin1Char('/');
    QDir().mkpath(path);

    const auto fileName = File::normalizeDocumentFileName(JsonLdDocument::readProperty(info, "name").toString());
    auto normalizedDocInfo = info;
    JsonLdDocument::writeProperty(normalizedDocInfo, "name", fileName);

    if (!QFile::copy(filePath, path + fileName)) {
        qCWarning(Log) << "Failed to copy document from" << filePath << "to" << path << fileName;
        // TODO error message for the ui
        return;
    }

    QFile metaFile(path + QLatin1StringView("meta.json"));
    if (!metaFile.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store document meta data" << path << metaFile.errorString();
        // TODO error message for the ui
        return;
    }
    metaFile.write(JsonIO::write(JsonLdDocument::toJson(normalizedDocInfo)));
    metaFile.close();
    Q_EMIT documentAdded(id);
}

void DocumentManager::removeDocument(const QString &id)
{
    const QString path = basePath() + id;
    QDir docDir(path);
    if (!docDir.removeRecursively()) {
        qCWarning(Log) << "Failed to delete directory" << path;
    }
    Q_EMIT documentRemoved(id);
}

QString DocumentManager::basePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/documents/");
}

#include "moc_documentmanager.cpp"
