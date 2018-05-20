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

#include "countryinformation.h"

#include <KLocalizedString>

#include <QDebug>

using namespace KItinerary;

CountryInformation::CountryInformation()
{
}

CountryInformation::~CountryInformation() = default;

bool CountryInformation::operator==(const CountryInformation& other) const
{
    const auto dsEqual = m_drivingSide == other.m_drivingSide || m_drivingSide == KnowledgeDb::DrivingSide::Unknown || other.m_drivingSide == KnowledgeDb::DrivingSide::Unknown;
    const auto ppEqual = (m_powerPlugs & other.m_powerPlugs) || m_powerPlugs == KnowledgeDb::Unknown || other.m_powerPlugs == KnowledgeDb::Unknown;

    return dsEqual && ppEqual;
}

QString CountryInformation::isoCode() const
{
    return m_isoCode;
}

void CountryInformation::setIsoCode(const QString& isoCode)
{
    if (m_isoCode == isoCode) {
        return;
    }
    m_isoCode = isoCode;

    const auto id = KnowledgeDb::CountryId{isoCode};
    if (!id.isValid()) {
        return;
    }
    const auto countryRecord = KnowledgeDb::countryForId(id);
    setDrivingSide(countryRecord.drivingSide);
    setPowerPlugTypes(countryRecord.powerPlugTypes);
}

KnowledgeDb::DrivingSide CountryInformation::drivingSide() const
{
    return m_drivingSide;
}

void CountryInformation::setDrivingSide(KnowledgeDb::DrivingSide drivingSide)
{
    if (m_drivingSide == drivingSide || drivingSide == KnowledgeDb::DrivingSide::Unknown) {
        return;
    }

    if (m_drivingSide != KnowledgeDb::DrivingSide::Unknown) {
        m_drivingSideDiffers = true;
    }

    m_drivingSide = drivingSide;
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

QString CountryInformation::powerPlugTypes() const
{
    QStringList l;
    for (const auto &elem : plug_name_table) {
        if (m_powerPlugs & elem.type) {
            l.push_back(i18n(elem.name));
        }
    }
    return l.join(QLatin1String(", "));
}

void CountryInformation::setPowerPlugTypes(KItinerary::KnowledgeDb::PowerPlugTypes powerPlugs)
{
    // TODO deal with partial incompatibilities and compatibilities between plug types
    if (m_powerPlugs == powerPlugs || powerPlugs == KnowledgeDb::Unknown) {
        return;
    }

    if ((m_powerPlugs & powerPlugs) == 0) {
        m_powerPlugTypesDiffer = true;
    }

    m_powerPlugs = powerPlugs;
}

bool CountryInformation::drivingSideDiffers() const
{
    return m_drivingSideDiffers;
}

bool CountryInformation::powerPlugTypesDiffer() const
{
    return m_powerPlugTypesDiffer;
}
