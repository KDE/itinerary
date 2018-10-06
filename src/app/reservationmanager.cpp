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
#include "pkpassmanager.h"
#include "logging.h"

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>

#include <QDate>
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

void ReservationManager::setPkPassManager(PkPassManager* mgr)
{
    m_passMgr = mgr;
    connect(mgr, &PkPassManager::passAdded, this, &ReservationManager::passAdded);
    connect(mgr, &PkPassManager::passUpdated, this, &ReservationManager::passUpdated);
    connect(mgr, &PkPassManager::passRemoved, this, &ReservationManager::passRemoved);
}

QVector<QString> ReservationManager::reservations() const
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations");
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
    if (id.isEmpty()) {
        return {};
    }

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

    // re-run post-processing to benefit from newer augmentations
    ExtractorPostprocessor postproc;
    postproc.process(resData);
    if (postproc.result().size() != 1) {
        qCWarning(Log) << "Post-processing discarded the reservation:" << resPath;
        return {};
    }

    const auto res = postproc.result().at(0);
    m_reservations.insert(id, res);
    return res;
}

void ReservationManager::importReservation(const QByteArray& data)
{
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data, &error);
    if (!doc.isArray()) {
        qCWarning(Log) << "Invalid JSON format." << error.errorString() << error.offset;
        return;
    }

    const auto resData = JsonLdDocument::fromJson(doc.array());
    importReservations(resData);
}

void ReservationManager::importReservations(const QVector<QVariant> &resData)
{
    ExtractorPostprocessor postproc;
    postproc.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    postproc.process(resData);

    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/");
    QDir::root().mkpath(basePath);

    for (auto res : postproc.result()) {
        QString resId;
        bool oldResFound = false;

        // check if we know this one already, and update if that's the case
        for (const auto &oldResId : reservations()) {
            const auto oldRes = reservation(oldResId);
            if (MergeUtil::isSame(oldRes, res)) {
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

void ReservationManager::addReservation(const QVariant &res)
{
    QString resId = QUuid::createUuid().toString();
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/");
    QDir::root().mkpath(basePath);
    const QString path = basePath + resId + QLatin1String(".jsonld");
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Unable to create file:" << f.errorString();
        return;
    }
    f.write(QJsonDocument(JsonLdDocument::toJson({res})).toJson());
    m_reservations.insert(resId, res);
    emit reservationAdded(resId);
}

void ReservationManager::updateReservation(const QString &resId, const QVariant &res)
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/");
    QDir::root().mkpath(basePath);
    const QString path = basePath + resId + QLatin1String(".jsonld");
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Unable to open file:" << f.errorString();
        return;
    }
    f.write(QJsonDocument(JsonLdDocument::toJson({res})).toJson());
    m_reservations.insert(resId, res);
    emit reservationUpdated(resId);
}

void ReservationManager::removeReservation(const QString& id)
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reservations/");
    QFile::remove(basePath + QLatin1Char('/') + id + QLatin1String(".jsonld"));
    emit reservationRemoved(id);
    m_reservations.remove(id);
}

void ReservationManager::removeReservations(const QStringList& ids)
{
    for (const auto &id : ids)
        removeReservation(id);
}

void ReservationManager::passAdded(const QString& passId)
{
    const auto pass = m_passMgr->pass(passId);
    ExtractorEngine engine;
    engine.setPass(pass);
    const auto data = engine.extract();
    const auto res = JsonLdDocument::fromJson(data);
    importReservations(res);
}

void ReservationManager::passUpdated(const QString& passId)
{
    passAdded(passId);
}

void ReservationManager::passRemoved(const QString& passId)
{
    Q_UNUSED(passId);
    // TODO
}
