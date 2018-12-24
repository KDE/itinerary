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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "manager.h"
#include "departurereply.h"
#include "departurerequest.h"
#include "journeyreply.h"
#include "journeyrequest.h"
#include "locationreply.h"
#include "locationrequest.h"
#include "logging.h"

#include <KPublicTransport/Location>

#include "backends/cache.h"
#include "backends/hafasmgatebackend.h"
#include "backends/navitiabackend.h"

#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>
#include <QNetworkAccessManager>

using namespace KPublicTransport;

static inline void initResources() {
    Q_INIT_RESOURCE(networks);
}

namespace KPublicTransport {
class ManagerPrivate {
public:
    QNetworkAccessManager* nam();
    void loadNetworks();
    std::unique_ptr<AbstractBackend> loadNetwork(const QJsonObject &obj);
    template <typename T> std::unique_ptr<AbstractBackend> loadNetwork(const QJsonObject &obj);

    QNetworkAccessManager *m_nam = nullptr;
    std::vector<std::unique_ptr<AbstractBackend>> m_backends;
    bool m_allowInsecure = false;
};
}

QNetworkAccessManager* ManagerPrivate::nam()
{
    if (!m_nam) {
        m_nam = new QNetworkAccessManager;
    }
    return m_nam;
}

void ManagerPrivate::loadNetworks()
{
    QDirIterator it(QStringLiteral(":/org.kde.pim/kpublictransport/networks"));
    while (it.hasNext()) {
        QFile f(it.next());
        if (!f.open(QFile::ReadOnly)) {
            qCWarning(Log) << "Failed to open public transport network configuration:" << f.errorString();
            continue;
        }

        QJsonParseError error;
        const auto doc = QJsonDocument::fromJson(f.readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(Log) << "Failed to parse public transport network configuration:" << error.errorString() << it.fileName();
            continue;
        }

        auto net = loadNetwork(doc.object());
        if (net) {
            net->setBackendId(it.fileInfo().baseName());
            m_backends.push_back(std::move(net));
        } else {
            qCWarning(Log) << "Failed to load public transport network configuration config:" << it.fileName();
        }
    }

    qCDebug(Log) << m_backends.size() << "public transport network configurations loaded";
}

std::unique_ptr<AbstractBackend> ManagerPrivate::loadNetwork(const QJsonObject &obj)
{
    const auto type = obj.value(QLatin1String("type")).toString();
    if (type == QLatin1String("navitia")) {
        return loadNetwork<NavitiaBackend>(obj);
    } else if (type == QLatin1String("hafas_mgate")) {
        return loadNetwork<HafasMgateBackend>(obj);
    }

    qCWarning(Log) << "Unknown backend type:" << type;
    return {};
}

static void applyBackendOptions(AbstractBackend *backend, const QMetaObject *mo, const QJsonObject &obj)
{
    const auto opts = obj.value(QLatin1String("options")).toObject();
    for (auto it = opts.begin(); it != opts.end(); ++it) {
        const auto idx = mo->indexOfProperty(it.key().toUtf8());
        const auto mp = mo->property(idx);
        if (it.value().isObject()) {
            mp.writeOnGadget(backend, it.value().toObject());
        } else {
            mp.writeOnGadget(backend, it.value().toVariant());
        }
    }

    const auto filter = obj.value(QLatin1String("filter")).toObject();
    const auto geoFilter = filter.value(QLatin1String("geo")).toArray();
    if (!geoFilter.isEmpty()) {
        QPolygonF poly;
        poly.reserve(geoFilter.size());
        for (const auto &coordV : geoFilter) {
            const auto coordA = coordV.toArray();
            poly.push_back({coordA[0].toDouble(), coordA[1].toDouble()});
        }
        backend->setGeoFilter(poly);
    }
}

template<typename T> std::unique_ptr<AbstractBackend> ManagerPrivate::loadNetwork(const QJsonObject &obj)
{
    std::unique_ptr<AbstractBackend> backend(new T);
    applyBackendOptions(backend.get(), &T::staticMetaObject, obj);
    return backend;
}

Manager::Manager() :
    d(new ManagerPrivate)
{
    initResources();
    d->loadNetworks();
}

Manager::Manager(Manager&&) noexcept = default;
Manager::~Manager() = default;

void Manager::setNetworkAccessManager(QNetworkAccessManager *nam)
{
    // TODO delete d->nam if we created it ourselves
    d->m_nam = nam;
}

void Manager::setAllowInsecureBackends(bool insecure)
{
    d->m_allowInsecure = insecure;
}

JourneyReply* Manager::queryJourney(const JourneyRequest &req) const
{
    auto reply = new JourneyReply(req);
    int pendingOps = 0;
    for (const auto &backend : d->m_backends) {
        if (backend->isLocationExcluded(req.from()) && backend->isLocationExcluded(req.to())) {
            qCDebug(Log) << "Skiping backend based on location filter:" << backend->backendId();
            continue;
        }
        if (!backend->isSecure() && !d->m_allowInsecure) {
            qCDebug(Log) << "Skipping insecure backend:" << backend->backendId();
            continue;
        }
        if (backend->queryJourney(reply, d->nam())) {
            ++pendingOps;
        }
    }
    reply->setPendingOps(pendingOps);
    return reply;
}

DepartureReply* Manager::queryDeparture(const DepartureRequest &req) const
{
    auto reply = new DepartureReply(req);
    int pendingOps = 0;
    for (const auto &backend : d->m_backends) {
        if (backend->isLocationExcluded(req.stop())) {
            qCDebug(Log) << "Skiping backend based on location filter:" << backend->backendId();
            continue;
        }
        if (!backend->isSecure() && !d->m_allowInsecure) {
            qCDebug(Log) << "Skipping insecure backend:" << backend->backendId();
            continue;
        }
        if (backend->queryDeparture(reply, d->nam())) {
            ++pendingOps;
        }
    }
    reply->setPendingOps(pendingOps);
    return reply;
}

LocationReply* Manager::queryLocation(const LocationRequest &req) const
{
    auto reply = new LocationReply(req);
    int pendingOps = 0;
    for (const auto &backend : d->m_backends) {
        if (req.hasCoordinate() && backend->isCoordinateExcluded(req.latitude(), req.longitude())) {
            qCDebug(Log) << "Skiping backend based on location filter:" << backend->backendId();
            continue;
        }
        if (!backend->isSecure() && !d->m_allowInsecure) {
            qCDebug(Log) << "Skipping insecure backend:" << backend->backendId();
            continue;
        }

        auto cache = Cache::lookupLocation(backend->backendId(), req.cacheKey());
        switch (cache.type) {
            case CacheHitType::Negative:
                qCDebug(Log) << "Negative cache hit for backend" << backend->backendId();
                break;
            case CacheHitType::Positive:
                qCDebug(Log) << "Positive cache hit for backend" << backend->backendId();
                reply->addResult(std::move(cache.data));
                break;
            case CacheHitType::Miss:
                qCDebug(Log) << "Cache miss for backend" << backend->backendId();
                if (backend->queryLocation(reply, d->nam())) {
                    ++pendingOps;
                }
                break;
        }
    }
    reply->setPendingOps(pendingOps);
    return reply;
}
