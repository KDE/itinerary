/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "documentmanager.h"
#include "logging.h"

#include <KItinerary/CreativeWork>
#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QVariant>

using namespace KItinerary;

DocumentManager* DocumentManager::s_instance = nullptr;

DocumentManager::DocumentManager(QObject* parent)
    : QObject(parent)
{
    s_instance = this;
}

DocumentManager::~DocumentManager()
{
    s_instance = nullptr;
}

DocumentManager* DocumentManager::instance()
{
    return s_instance;
}

QVector<QString> DocumentManager::documents() const
{
    QVector<QString> docs;

    QDirIterator it(basePath(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        docs.push_back(it.fileName());
    }

    return docs;
}

bool DocumentManager::hasDocument(const QString& id) const
{
    return QDir(basePath() + id).exists();
}

QVariant DocumentManager::documentInfo(const QString &id) const
{
    const QString filePath = basePath() + id + QLatin1String("/meta.json");
    QFile f(filePath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to load document meta data" << filePath << f.errorString();
        return {};
    }
    return JsonLdDocument::fromJson(QJsonDocument::fromJson(f.readAll()).object());
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

    QFile metaFile(path + QLatin1String("meta.json"));
    if (!metaFile.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store document meta data" << path << metaFile.errorString();
        // TODO error message for the ui
        return;
    }
    metaFile.write(QJsonDocument(JsonLdDocument::toJson(normalizedDocInfo)).toJson());
    emit documentAdded(id);
}

void DocumentManager::addDocument(const QString& id, const QVariant& info, const QString& filePath)
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

    QFile metaFile(path + QLatin1String("meta.json"));
    if (!metaFile.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store document meta data" << path << metaFile.errorString();
        // TODO error message for the ui
        return;
    }
    metaFile.write(QJsonDocument(JsonLdDocument::toJson(normalizedDocInfo)).toJson());
    emit documentAdded(id);
}

void DocumentManager::removeDocument(const QString& id)
{
    const QString path = basePath() + id;
    QDir docDir(path);
    if (!docDir.removeRecursively()) {
        qCWarning(Log) << "Failed to delete directory" << path;
    }
    emit documentRemoved(id);
}

QString DocumentManager::basePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/documents/");
}
