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

    Q_PROPERTY(PowerPlugCompatibility powerPlugCompatibility READ powerPlugCompatibility)
    /** Plugs from the home country that will not fit. */
    Q_PROPERTY(QString powerPlugTypes READ powerPlugTypes)
    /** Sockets in the destination country that are incompatible with (some of) my plugs. */
    Q_PROPERTY(QString powerSocketTypes READ powerSocketTypes)

public:
    CountryInformation();
    ~CountryInformation();

    enum PowerPlugCompatibility {
        FullyCompatible,
        PartiallyCompatible,
        Incompatible
    };
    Q_ENUM(PowerPlugCompatibility)

    bool operator==(const CountryInformation &other) const;

    QString isoCode() const;
    void setIsoCode(const QString &isoCode);

    KItinerary::KnowledgeDb::DrivingSide drivingSide() const;
    bool drivingSideDiffers() const;

    PowerPlugCompatibility powerPlugCompatibility() const;
    QString powerPlugTypes() const;
    QString powerSocketTypes() const;

private:
    void setDrivingSide(KItinerary::KnowledgeDb::DrivingSide drivingSide);
    void setPowerPlugTypes(KItinerary::KnowledgeDb::PowerPlugTypes powerPlugs);

    QString m_isoCode;
    KItinerary::KnowledgeDb::PowerPlugTypes m_powerPlugs = KItinerary::KnowledgeDb::Unknown;
    KItinerary::KnowledgeDb::PowerPlugTypes m_incompatPlugs = KItinerary::KnowledgeDb::Unknown;
    KItinerary::KnowledgeDb::PowerPlugTypes m_incompatSockets = KItinerary::KnowledgeDb::Unknown;
    KItinerary::KnowledgeDb::DrivingSide m_drivingSide = KItinerary::KnowledgeDb::DrivingSide::Unknown;
    bool m_drivingSideDiffers = false;
    PowerPlugCompatibility m_powerPlugCompat = FullyCompatible;
};

Q_DECLARE_METATYPE(CountryInformation)

#endif // COUNTRYINFORMATION_H
