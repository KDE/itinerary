/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deutschebahnnextcheckin.h"
#include "logging.h"
#include "onlineticketretrievaljob.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Uic9183Header>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/Vendor0080Block>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

std::function<QNetworkAccessManager*()> DeutscheBahnNextCheckIn::s_namFactory;

DeutscheBahnNextCheckIn::DeutscheBahnNextCheckIn(QObject *parent)
    : QObject(parent)
{
}

DeutscheBahnNextCheckIn::~DeutscheBahnNextCheckIn() = default;

void DeutscheBahnNextCheckIn::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory)
{
    s_namFactory = namFactory;
}

bool DeutscheBahnNextCheckIn::checkInAvailable(const QVariant &res) const
{
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();

    KItinerary::Uic9183Parser uic918;
    uic918.parse(trainTicket.ticketTokenData().toByteArray());
    return uic918.header().signerCompanyCode() == QLatin1String("1080");
}

bool DeutscheBahnNextCheckIn::checkInEnabled(const QVariant &res) const
{
    // TODO we need the 12 digit order number (which is not in the barcode!)
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTrip = trainRes.reservationFor().value<KItinerary::TrainTrip>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();
    const auto underName = trainRes.underName().value<KItinerary::Person>();

    return !trainTicket.ticketedSeat().seatNumber().isEmpty()
        && !trainTicket.ticketedSeat().seatSection().isEmpty()
        && !underName.familyName().isEmpty()
        && !trainRes.programMembershipUsed().membershipNumber().isEmpty(); // TODO only when needed
        // TODO check train type/number
        // TODO trip is current
}

void DeutscheBahnNextCheckIn::checkIn(const QString &resId)
{
    if (!m_resMgr) {
        qCWarning(Log) << "ReservationManager not set!?";
        return;
    }

    doAuthenticate(resId);
    // TODO step 1 authenticate
    // TODO step 2 order request
    // TODO step 3 checkin command
}

void DeutscheBahnNextCheckIn::doAuthenticate(const QString &resId)
{
    // TODO we need the order number in trainRes.reservationNumber
    const auto trainRes = m_resMgr->reservation(resId).value<KItinerary::TrainReservation>();
    const auto orderNumber = QLatin1String(""); // TODO
    const auto name = trainRes.underName().value<KItinerary::Person>().familyName();

    QNetworkRequest req(QUrl(QStringLiteral("https://www.bahn.de/web/api/buchung/auftrag/authenticate")));
    req.setAttribute(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QJsonObject reqData({
        {QLatin1String("auftragsnummer"), orderNumber},
        {QLatin1String("nachname") , name}
    });
    auto reply = s_namFactory()->post(req, QJsonDocument(reqData).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [reply, this]() {
        // TODO this doesn't seem to work (always 500, presumably missing hcaptcha data?)
        reply->deleteLater();
        qWarning() << reply->readAll() << reply->errorString();
    });
}

#include "moc_deutschebahnnextcheckin.cpp"
