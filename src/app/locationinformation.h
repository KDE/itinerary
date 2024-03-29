/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOCATIONINFORMATION_H
#define LOCATIONINFORMATION_H

#include <KItinerary/CountryDb>

#include <QMetaType>
#include <QString>
#include <QTimeZone>

/** Data for country information elements in the timeline model. */
class LocationInformation
{
    Q_GADGET
    Q_PROPERTY(QString isoCode READ isoCode)

    /** @warning Careful with using this in QML, this is an 8bit enum which QML doesn't seem to like
     *  and you might silently get arbitrary values instead.
     */
    Q_PROPERTY(KItinerary::KnowledgeDb::DrivingSide drivingSide READ drivingSide)
    /** This indicates that the driving side information changed and needs to be displayed. */
    Q_PROPERTY(bool drivingSideDiffers READ drivingSideDiffers)
    /** Display label for the driving side information. */
    Q_PROPERTY(QString drivingSideLabel READ drivingSideLabel STORED false)

    Q_PROPERTY(PowerPlugCompatibility powerPlugCompatibility READ powerPlugCompatibility)
    /** Plugs from the home country that will not fit. */
    Q_PROPERTY(QString powerPlugTypes READ powerPlugTypes)
    /** Sockets in the destination country that are incompatible with (some of) my plugs. */
    Q_PROPERTY(QString powerSocketTypes READ powerSocketTypes)

    Q_PROPERTY(bool timeZoneDiffers READ timeZoneDiffers)
    Q_PROPERTY(QString timeZoneName READ timeZoneName)
    Q_PROPERTY(int timeZoneOffsetDelta READ timeZoneOffsetDelta)

    Q_PROPERTY(bool currencyDiffers READ currencyDiffers)
    Q_PROPERTY(QString currencyCode READ currencyCode)

public:
    LocationInformation();
    ~LocationInformation();

    enum PowerPlugCompatibility {
        FullyCompatible,
        PartiallyCompatible,
        Incompatible
    };
    Q_ENUM(PowerPlugCompatibility)

    bool operator==(const LocationInformation &other) const;

    QString isoCode() const;
    void setIsoCode(const QString &isoCode);

    KItinerary::KnowledgeDb::DrivingSide drivingSide() const;
    bool drivingSideDiffers() const;
    QString drivingSideLabel() const;

    PowerPlugCompatibility powerPlugCompatibility() const;
    QString powerPlugTypes() const;
    QString powerSocketTypes() const;

    QTimeZone timeZone() const;
    QDateTime transitionTime() const;
    void setTimeZone(const QTimeZone &tz, const QDateTime &transitionTime);
    bool hasRelevantTimeZoneChange(const LocationInformation &other) const;
    bool timeZoneDiffers() const;
    QString timeZoneName() const;
    int timeZoneOffsetDelta() const;

    bool currencyDiffers() const;
    QString currencyCode() const;

private:
    void setDrivingSide(KItinerary::KnowledgeDb::DrivingSide drivingSide);
    void setPowerPlugTypes(KItinerary::KnowledgeDb::PowerPlugTypes powerPlugs);

    QString m_isoCode;
    QTimeZone m_timeZone;
    QDateTime m_transitionTime;
    QString m_currency;
    KItinerary::KnowledgeDb::PowerPlugTypes m_powerPlugs = KItinerary::KnowledgeDb::Unknown;
    KItinerary::KnowledgeDb::PowerPlugTypes m_incompatPlugs = KItinerary::KnowledgeDb::Unknown;
    KItinerary::KnowledgeDb::PowerPlugTypes m_incompatSockets = KItinerary::KnowledgeDb::Unknown;
    KItinerary::KnowledgeDb::DrivingSide m_drivingSide = KItinerary::KnowledgeDb::DrivingSide::Unknown;
    bool m_drivingSideDiffers = false;
    PowerPlugCompatibility m_powerPlugCompat = FullyCompatible;
    int m_timeZoneOffsetDelta = 0;
    bool m_currencyDiffers = false;
};

Q_DECLARE_METATYPE(LocationInformation)

#endif // LOCATIONINFORMATION_H
