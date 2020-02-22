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

#include "locationinformation.h"

#include <KLocalizedString>

#include <QDebug>

using namespace KItinerary;

LocationInformation::LocationInformation()
{
}

LocationInformation::~LocationInformation() = default;

bool LocationInformation::operator==(const LocationInformation& other) const
{
    const auto dsEqual = m_drivingSide == other.m_drivingSide || m_drivingSide == KnowledgeDb::DrivingSide::Unknown || other.m_drivingSide == KnowledgeDb::DrivingSide::Unknown;
    const auto ppEqual = (m_incompatPlugs == other.m_incompatPlugs && m_incompatSockets == other.m_incompatSockets)
                        || m_powerPlugs == KnowledgeDb::Unknown || other.m_powerPlugs == KnowledgeDb::Unknown;

    return dsEqual && ppEqual;
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
