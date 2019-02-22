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

#ifndef KPUBLICTRANSPORT_CACHE_H
#define KPUBLICTRANSPORT_CACHE_H

#include "kpublictransport_export.h"

#include <vector>

class QString;

namespace KPublicTransport {

class Location;

enum class CacheHitType {
    Miss,
    Positive,
    Negative
};

template <typename T> struct CacheEntry
{
    CacheEntry() = default;
    CacheEntry(CacheEntry&&) = default;
    CacheEntry(const CacheEntry&) = delete;
    ~CacheEntry() = default;
    CacheEntry& operator=(CacheEntry&&) = default;
    CacheEntry& operator=(const CacheEntry&) = delete;

    std::vector<T> data;
    CacheHitType type = CacheHitType::Miss;
};

/** Query result caching functions.
 *  @internal exported for unit tests only
 */
namespace Cache
{
    /** Add data to the cache. */
    KPUBLICTRANSPORT_EXPORT void addLocationCacheEntry(const QString &backendId, const QString &cacheKey, const std::vector<Location> &data);
    /** Add negative cache entry for location queries, ie. remember a result could not be found. */
    KPUBLICTRANSPORT_EXPORT void addNegativeLocationCacheEntry(const QString &backendId, const QString &cacheKey);
    /** Perform cache lookuip for location results. */
    KPUBLICTRANSPORT_EXPORT CacheEntry<Location> lookupLocation(const QString &backendId, const QString &cacheKey);

    /** Expire old cache entries. */
    void KPUBLICTRANSPORT_EXPORT expire();
}

}

#endif // KPUBLICTRANSPORT_CACHE_H
