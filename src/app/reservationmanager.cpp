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

#include "reservationmanager.h"
#include "logging.h"

#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QUrl>
#include <QUuid>
#include <QVector>

using namespace KItinerary;

ReservationManager::ReservationManager(QObject* parent)
    : QObject(parent)
{
}

ReservationManager::~ReservationManager() = default;

QVector<QString> ReservationManager::reservations() const
{
    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations");
    QDir::root().mkpath(basePath);

    QVector<QString> resIds;
    for (QDirIterator it(basePath, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        resIds.push_back(it.fileInfo().baseName());
    }

    return resIds;
}

QVariant ReservationManager::reservation(const QString& id) const
{
    const auto it = m_reservations.constFind(id);
    if (it != m_reservations.constEnd()) {
        return it.value();
    }

    const QString resPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/") + id + QLatin1String(".jsonld");
    QFile f(resPath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open JSON-LD reservation data file:" << resPath << f.errorString();
        return {};
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray() && doc.array().size() != 1) {
        qCWarning(Log) << "Invalid JSON-LD reservation data file:" << resPath;
        return {};
    }

    const auto resData = JsonLdDocument::fromJson(doc.array());
    if (resData.size() != 1) {
        qCWarning(Log) << "Unable to parse JSON-LD reservation data file:" << resPath;
        return {};
    }

    m_reservations.insert(id, resData.at(0));
    return resData.at(0);
}

void ReservationManager::importReservation(const QUrl& filename)
{
    if (!filename.isLocalFile())
        return;

    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/");
    QDir::root().mkpath(basePath);

    QFile f(filename.toLocalFile());
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Unable to open file:" << f.errorString();
        return;
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) {
        qCWarning(Log) << "Invalid JSON format.";
        return;
    }

    const auto resData = JsonLdDocument::fromJson(doc.array());
    for (auto res : resData) {
        QString resId;
        bool oldResFound = false;

        // check if we know this one already, and update if that's the case
        for (const auto &oldResId : reservations()) {
            const auto oldRes = reservation(oldResId);
            if (MergeUtil::isSameReservation(oldRes, res)) {
                res = JsonLdDocument::apply(oldRes, res);
                resId = oldResId;
                oldResFound = true;
                break;
            }
        }

        if (resId.isEmpty()) {
            resId = QUuid::createUuid().toString();
        }

        const QString path = basePath + resId + QLatin1String(".jsonld");
        QFile f(path);
        if (!f.open(QFile::WriteOnly)) {
            qCWarning(Log) << "Unable to create file:" << f.errorString();
            continue;
        }
        f.write(QJsonDocument(JsonLdDocument::toJson({res})).toJson());
        m_reservations.insert(resId, res);

        if (oldResFound) {
            emit reservationUpdated(resId);
        } else {
            emit reservationAdded(resId);
        }
    }
}

void ReservationManager::removeReservation(const QString& id)
{
    const auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/");
    QFile::remove(basePath + QLatin1Char('/') + id + QLatin1String(".jsonld"));
    m_reservations.remove(id);
    emit reservationRemoved(id);
}
