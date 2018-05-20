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

#include <QDebug>

using namespace KItinerary;

CountryInformation::CountryInformation()
{
}

CountryInformation::~CountryInformation() = default;

bool CountryInformation::operator==(const CountryInformation& other) const
{
    return m_drivingSide == other.m_drivingSide || m_drivingSide == KnowledgeDb::DrivingSide::Unknown || other.m_drivingSide == KnowledgeDb::DrivingSide::Unknown;
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

bool CountryInformation::drivingSideDiffers() const
{
    return m_drivingSideDiffers;
}
