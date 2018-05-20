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

#ifndef COUNTRYINFORMATION_H
#define COUNTRYINFORMATION_H

#include <KItinerary/CountryDb>

#include <QMetaType>
#include <QString>

/** Data for country information elements in the timeline model. */
class CountryInformation
{
    Q_GADGET
    Q_PROPERTY(QString isoCode READ isoCode)
    Q_PROPERTY(KItinerary::KnowledgeDb::DrivingSide drivingSide READ drivingSide)
    /** This indicates that the driving side information changed and needs to be displayed. */
    Q_PROPERTY(bool drivingSideDiffers READ drivingSideDiffers)
public:
    CountryInformation();
    ~CountryInformation();

    bool operator==(const CountryInformation &other) const;

    QString isoCode() const;
    void setIsoCode(const QString &isoCode);

    KItinerary::KnowledgeDb::DrivingSide drivingSide() const;
    bool drivingSideDiffers() const;

private:
    void setDrivingSide(KItinerary::KnowledgeDb::DrivingSide drivingSide);

    QString m_isoCode;
    KItinerary::KnowledgeDb::DrivingSide m_drivingSide = KItinerary::KnowledgeDb::DrivingSide::Unknown;
    bool m_drivingSideDiffers = false;
};

Q_DECLARE_METATYPE(CountryInformation)

#endif // COUNTRYINFORMATION_H
