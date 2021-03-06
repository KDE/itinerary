/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationinformation.h"

#include <KLocalizedString>

#include <QDebug>

using namespace KItinerary;

LocationInformation::LocationInformation() = default;
LocationInformation::~LocationInformation() = default;

bool LocationInformation::operator==(const LocationInformation& other) const
{
    const auto dsEqual = m_drivingSide == other.m_drivingSide || m_drivingSide == KnowledgeDb::DrivingSide::Unknown || other.m_drivingSide == KnowledgeDb::DrivingSide::Unknown;
    const auto ppEqual = (m_incompatPlugs == other.m_incompatPlugs && m_incompatSockets == other.m_incompatSockets)
                        || m_powerPlugs == KnowledgeDb::Unknown || other.m_powerPlugs == KnowledgeDb::Unknown;

    return dsEqual && ppEqual && !hasRelevantTimeZoneChange(other);
}

QString LocationInformation::isoCode() const
{
    return m_isoCode;
}

void LocationInformation::setIsoCode(const QString& isoCode)
{
    if (m_isoCode == isoCode) {
        return;
    }
    m_isoCode = isoCode;

    const auto id = KnowledgeDb::CountryId{isoCode};
    if (!id.isValid()) {
        setDrivingSide(KnowledgeDb::DrivingSide::Unknown);
        setPowerPlugTypes(KnowledgeDb::Unknown);
        return;
    }
    const auto countryRecord = KnowledgeDb::countryForId(id);
    setDrivingSide(countryRecord.drivingSide);
    setPowerPlugTypes(countryRecord.powerPlugTypes);
}

KnowledgeDb::DrivingSide LocationInformation::drivingSide() const
{
    return m_drivingSide;
}

void LocationInformation::setDrivingSide(KnowledgeDb::DrivingSide drivingSide)
{
    if (m_drivingSide == drivingSide) {
        return;
    }

    if (m_drivingSide != KnowledgeDb::DrivingSide::Unknown) {
        m_drivingSideDiffers = true;
    }

    m_drivingSide = drivingSide;
}

bool LocationInformation::drivingSideDiffers() const
{
    return m_drivingSideDiffers;
}

LocationInformation::PowerPlugCompatibility LocationInformation::powerPlugCompatibility() const
{
    return m_powerPlugCompat;
}

struct plugTypeName {
    KnowledgeDb::PowerPlugType type;
    const char *name;
}

static const plug_name_table[] = {
    { KnowledgeDb::TypeA, I18N_NOOP("Type A") },
    { KnowledgeDb::TypeB, I18N_NOOP("Type B") },
    { KnowledgeDb::TypeC, I18N_NOOP("Europlug") },
    { KnowledgeDb::TypeD, I18N_NOOP("Type D") },
    { KnowledgeDb::TypeE, I18N_NOOP("Type E") },
    { KnowledgeDb::TypeF, I18N_NOOP("Schuko") },
    { KnowledgeDb::TypeG, I18N_NOOP("Type G") },
    { KnowledgeDb::TypeH, I18N_NOOP("Type H") },
    { KnowledgeDb::TypeI, I18N_NOOP("Type I") },
    { KnowledgeDb::TypeJ, I18N_NOOP("Type J") },
    { KnowledgeDb::TypeK, I18N_NOOP("Type K") },
    { KnowledgeDb::TypeL, I18N_NOOP("Type L") },
    { KnowledgeDb::TypeM, I18N_NOOP("Type M") },
    { KnowledgeDb::TypeN, I18N_NOOP("Type N") },
};

static QString plugTypesToString(KnowledgeDb::PowerPlugTypes type)
{
    QStringList l;
    for (const auto &elem : plug_name_table) {
        if (type & elem.type) {
            l.push_back(i18n(elem.name));
        }
    }
    return l.join(QLatin1String(", "));
}

QString LocationInformation::powerPlugTypes() const
{
    return plugTypesToString(m_incompatPlugs);
}

QString LocationInformation::powerSocketTypes() const
{
    return plugTypesToString(m_incompatSockets);
}

void LocationInformation::setPowerPlugTypes(KItinerary::KnowledgeDb::PowerPlugTypes powerPlugs)
{
    if (m_powerPlugs == powerPlugs) {
        return;
    }

    if (powerPlugs != KnowledgeDb::Unknown && m_powerPlugs != KnowledgeDb::Unknown) {
        m_incompatPlugs = KnowledgeDb::incompatiblePowerPlugs(m_powerPlugs, powerPlugs);
        m_incompatSockets = KnowledgeDb::incompatiblePowerSockets(m_powerPlugs, powerPlugs);

        if ((m_powerPlugs & powerPlugs) == 0) {
            m_powerPlugCompat = Incompatible;
        } else if (m_incompatPlugs != KnowledgeDb::Unknown || m_incompatSockets != KnowledgeDb::Unknown) {
            m_powerPlugCompat = PartiallyCompatible;
        }
    }

    m_powerPlugs = powerPlugs;
}

QTimeZone LocationInformation::timeZone() const
{
    return m_timeZone;
}

QDateTime LocationInformation::transitionTime() const
{
    return m_transitionTime;
}

void LocationInformation::setTimeZone(const QTimeZone &tz, const QDateTime &transitionTime)
{
    if (m_timeZone.isValid() && tz.isValid()) {
        m_timeZoneOffsetDelta = tz.offsetFromUtc(transitionTime) - m_timeZone.offsetFromUtc(transitionTime);
    } else {
        m_timeZoneOffsetDelta = 0;
    }
    m_timeZone = tz;
    m_transitionTime = transitionTime;
}

bool LocationInformation::hasRelevantTimeZoneChange(const LocationInformation &other) const
{
    return m_timeZone.isValid() && other.m_timeZone.isValid()
        && m_timeZone.offsetFromUtc(m_transitionTime) != other.m_timeZone.offsetFromUtc(m_transitionTime);
}

bool LocationInformation::timeZoneDiffers() const
{
    return m_timeZoneOffsetDelta != 0 && m_timeZone.isValid();
}

QString LocationInformation::timeZoneName() const
{
    return m_timeZone.displayName(m_transitionTime, QTimeZone::LongName);
}

int LocationInformation::timeZoneOffsetDelta() const
{
    return m_timeZoneOffsetDelta;
}
