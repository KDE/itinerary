/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationinformation.h"

#include <KCountry>
#include <KLazyLocalizedString>
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
    const auto currencyEqual = m_currency == other.m_currency || m_currency.isEmpty() || other.m_currency.isEmpty();

    return dsEqual && ppEqual && !hasRelevantTimeZoneChange(other) && currencyEqual;
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

    auto currency = KCountry::fromAlpha2(isoCode).currencyCode();
    if (currency != m_currency && !m_currency.isEmpty() && !currency.isEmpty()) {
        m_currencyDiffers = true;
    }
    m_currency = currency;
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

QString LocationInformation::drivingSideLabel() const
{
    switch (m_drivingSide) {
        case KItinerary::KnowledgeDb::DrivingSide::Right: return i18n("People are driving on the right side.");
        case KItinerary::KnowledgeDb::DrivingSide::Left: return i18n("People are driving on the left side.");
        case KItinerary::KnowledgeDb::DrivingSide::Unknown: return {};
    }
    return {};
}

LocationInformation::PowerPlugCompatibility LocationInformation::powerPlugCompatibility() const
{
    return m_powerPlugCompat;
}

struct plugTypeName {
    KnowledgeDb::PowerPlugType type;
    const KLazyLocalizedString name;
}

static const plug_name_table[] = {
    { KnowledgeDb::TypeA, kli18n("Type A") },
    { KnowledgeDb::TypeB, kli18n("Type B") },
    { KnowledgeDb::TypeC, kli18n("Europlug") },
    { KnowledgeDb::TypeD, kli18n("Type D") },
    { KnowledgeDb::TypeE, kli18n("Type E") },
    { KnowledgeDb::TypeF, kli18n("Schuko") },
    { KnowledgeDb::TypeG, kli18n("Type G") },
    { KnowledgeDb::TypeH, kli18n("Type H") },
    { KnowledgeDb::TypeI, kli18n("Type I") },
    { KnowledgeDb::TypeJ, kli18n("Type J") },
    { KnowledgeDb::TypeK, kli18n("Type K") },
    { KnowledgeDb::TypeL, kli18n("Type L") },
    { KnowledgeDb::TypeM, kli18n("Type M") },
    { KnowledgeDb::TypeN, kli18n("Type N") },
};

static QString plugTypesToString(KnowledgeDb::PowerPlugTypes type)
{
    QStringList l;
    for (const auto &elem : plug_name_table) {
        if (type & elem.type) {
            l.push_back(KLocalizedString(elem.name).toString());
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

bool LocationInformation::currencyDiffers() const
{
    return m_currencyDiffers;
}

QString LocationInformation::currencyCode() const
{
    return m_currency;
}
