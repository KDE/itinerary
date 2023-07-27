/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deutschebahncheckin.h"
#include "logging.h"
#include "onlineticketretrievaljob.h"

#include <kitinerary_version.h>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Uic9183Head>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/Vendor0080Block>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

std::function<QNetworkAccessManager*()> DeutscheBahnCheckIn::s_namFactory;

DeutscheBahnCheckIn::DeutscheBahnCheckIn(QObject *parent)
    : QObject(parent)
{
}

DeutscheBahnCheckIn::~DeutscheBahnCheckIn() = default;

void DeutscheBahnCheckIn::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory)
{
    s_namFactory = namFactory;
}

static QString seatReservationNumber(const KItinerary::Ticket &ticket)
{
#if KITINERARY_VERSION > QT_VERSION_CHECK(5, 24, 40)
    auto res = ticket.ticketedSeat().identifier();
    res.remove(QLatin1Char(' '));
    res = res.left(12);
    if (std::any_of(res.begin(), res.end(), [](QChar c) { return !c.isDigit(); })) {
        return {};
    }
    return res;
#else
    return {};
#endif
}

bool DeutscheBahnCheckIn::checkInAvailable(const QVariant &res) const
{
#if KITINERARY_VERSION > QT_VERSION_CHECK(5, 24, 40)
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();

    KItinerary::Uic9183Parser uic918;
    uic918.parse(trainTicket.ticketTokenData().toByteArray());
    // TODO U_FLEX only tickets
    return uic918.findBlock<KItinerary::Uic9183Head>().issuerCompanyCodeNumeric() == 80;
#else
    return false;
#endif
}

bool DeutscheBahnCheckIn::checkInEnabled(const QVariant &res) const
{
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTrip = trainRes.reservationFor().value<KItinerary::TrainTrip>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();
    const auto underName = trainRes.underName().value<KItinerary::Person>();

    return !trainTicket.ticketedSeat().seatNumber().isEmpty()
        && !trainTicket.ticketedSeat().seatSection().isEmpty()
        && trainTrip.departureStation().identifier().startsWith(QLatin1String("ibnr:"))
        && trainTrip.arrivalStation().identifier().startsWith(QLatin1String("ibnr:"))
        && !underName.givenName().isEmpty()
        && !underName.familyName().isEmpty()
        && !trainRes.programMembershipUsed().membershipNumber().isEmpty(); // TODO only when needed
        // TODO check train type/number
        // TODO trip is current
}

static bool needsOnlineTicketRetrieval(const QVariant &res)
{
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();

    // check for "ot_nummer" and seat reservation number, TODO check their exact formats?
    return trainTicket.ticketNumber().isEmpty() || seatReservationNumber(trainTicket).isEmpty();
}

struct {
    const char code[3];
    int discount;
} static constexpr const bc_map[] = {
    { "49", 25 },
    { "19", 50 },
    { "78", 50 },
};

static QJsonObject makeCheckRequest(const QVariant &res)
{
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTrip = trainRes.reservationFor().value<KItinerary::TrainTrip>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();
    const auto underName = trainRes.underName().value<KItinerary::Person>();

    KItinerary::Uic9183Parser uic918;
    uic918.parse(trainTicket.ticketTokenData().toByteArray());

    const auto blBlock = uic918.findBlock<KItinerary::Vendor0080BLBlock>();
    const auto sub9s = blBlock.findSubBlock("009").toString().split(QLatin1Char('-')); // TODO input validation
    const auto numAdults = sub9s[0].toInt();
    const auto numChildren = blBlock.findSubBlock("012").toString().toInt();
    int discount = 0;
    for (const auto &bc : bc_map) {
        if (sub9s[2] == QLatin1String(bc.code)) {
            discount = bc.discount;
            break;
        }
    }

    // TODO multi-traveler
    QJsonArray seats({QJsonObject{
        {QLatin1String("platznr"), trainTicket.ticketedSeat().seatNumber()},
        {QLatin1String("wagennr"), trainTicket.ticketedSeat().seatSection()},
    }});

    QJsonArray discounts;
    QJsonArray bahnCards;
    if (const auto numDiscounts = sub9s[1].toInt(); numDiscounts) {
        discounts = QJsonArray({QJsonObject{
            {QLatin1String("rbs"), discount},
            {QLatin1String("anz"), numDiscounts},
        }});
        bahnCards = QJsonArray({QJsonObject{
            {QLatin1String("rbs"), discount},
            {QLatin1String("chk"), 0}, // no idea what this is
            {QLatin1String("nr"), trainRes.programMembershipUsed().membershipNumber()},
        }});
    }

    QRegularExpression trainRx(QStringLiteral("([A-Z]+) *(\\d+)$"));
    const auto trainMatch = trainRx.match(trainTrip.trainName() + QLatin1Char(' ') + trainTrip.trainNumber()); // TODO input validation

    QJsonObject req({
        {QLatin1String("sci_sci_rq"), QJsonObject({
            {QLatin1String("plaetze"), seats},
            {QLatin1String("bc_rabatts"), discounts},
            {QLatin1String("bcs"), bahnCards},
            {QLatin1String("abfahrt"), QJsonObject({
                {QLatin1String("ebhf_name"), trainTrip.departureStation().name()},
                {QLatin1String("ebhf_nr"), trainTrip.departureStation().identifier().mid(5)},
                {QLatin1String("eva_name"), trainTrip.departureStation().name()},
                {QLatin1String("eva_nr"), trainTrip.departureStation().identifier().mid(5)},
                {QLatin1String("zeit"), trainTrip.departureTime().toUTC().toString(Qt::ISODate)},
            })},
            {QLatin1String("ankunft"), QJsonObject({
                {QLatin1String("ebhf_name"), trainTrip.arrivalStation().name()},
                {QLatin1String("ebhf_nr"), trainTrip.arrivalStation().identifier().mid(5)},
                {QLatin1String("eva_name"), trainTrip.arrivalStation().name()},
                {QLatin1String("eva_nr"), trainTrip.arrivalStation().identifier().mid(5)},
                {QLatin1String("zeit"), trainTrip.arrivalTime().toUTC().toString(Qt::ISODate)},
            })},
            {QLatin1String("anz_erw"), numAdults},
            {QLatin1String("anz_kind"), numChildren},
            {QLatin1String("anz_res"), numAdults + numChildren},
            {QLatin1String("kl"), trainTicket.ticketedSeat().seatingType().toInt()},
            {QLatin1String("res"), seatReservationNumber(trainTicket)},
            {QLatin1String("ticket"), QJsonObject({
                {QLatin1String("issuer"), uic918.findBlock<KItinerary::Uic9183Head>().issuerCompanyCodeNumeric()}, // TODO U_FLEX only tickets?
                {QLatin1String("tkey"), uic918.findBlock<KItinerary::Uic9183Head>().ticketKey()}, // TODO U_FLEX only tickets?
                {QLatin1String("ot_nummer"), trainTicket.ticketNumber()}, // TODO input validation
                {QLatin1String("bcb_erforderlich"), QLatin1String("N")}, // TODO what is this??
                {QLatin1String("reisender_vorname"), underName.givenName()},
                {QLatin1String("reisender_nachname"), underName.familyName()},
            })},
            {QLatin1String("ver"), 1},
            {QLatin1String("zug"), QJsonObject({
                {QLatin1String("gat"), trainMatch.captured(1)},
                {QLatin1String("nr"), trainMatch.captured(2)},
            })},
        })},
    });

    return req;
}


void DeutscheBahnCheckIn::checkIn(const QString &resId)
{
    if (!m_resMgr) {
        qCWarning(Log) << "ReservationManager not set!?";
        return;
    }

    const auto res = m_resMgr->reservation(resId);
    const auto trainRes = res.value<KItinerary::TrainReservation>();
    const auto trainTicket = trainRes.reservedTicket().value<KItinerary::Ticket>();

    if (needsOnlineTicketRetrieval(res)) {
        qCDebug(Log) << "KCI needs online ticket retrieval";
        QVariantMap args;
        args.insert(QStringLiteral("reference"), trainRes.reservationNumber());
        args.insert(QStringLiteral("name"), trainRes.underName().value<KItinerary::Person>().familyName().toUpper());
        auto job = new OnlineTicketRetrievalJob(QStringLiteral("db"), args, s_namFactory(), this);
        connect(job, &OnlineTicketRetrievalJob::finished, this, [this, job, resId]() {
            job->deleteLater();
            if (job->result().isEmpty()) {
                // TODO error feedback
                qCWarning(Log) << job->errorMessage();
                return;
            }

            m_resMgr->importReservations(job->result());
            const auto res = m_resMgr->reservation(resId);
            // TODO overwrite IBNRs, the ones we get from online import are "more correct"
            doCheckIn(res);
        });
    } else {
        doCheckIn(res);
    }
}

void DeutscheBahnCheckIn::doCheckIn(const QVariant &res)
{
    // this should have happened by now, and if it didn't something failed...
    if (needsOnlineTicketRetrieval(res)) {
        // TODO error feedback
        qCWarning(Log) << "still incomplete data for KCI!";
        return;
    }

    const auto req = makeCheckRequest(res);
    // TODO
    qDebug().noquote() << QJsonDocument(req).toJson();
    return;

    QNetworkRequest netReq(QUrl(QStringLiteral("https://kanalbackend-navigator-prd-default-kci-tck.dbv.noncd.db.de/sci_sci")));
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json;charset=utf-8"));
    netReq.setRawHeader("Referer", "https://www.img-bahn.de/");
    netReq.setRawHeader("Origin", "https://www.img-bahn.de");
    auto reply = s_namFactory()->post(netReq, QJsonDocument(req).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            // TODO report network error
            qCWarning(Log) << reply->error() << reply->errorString() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            return;
        }

        const auto replyData = reply->readAll();
        qCDebug(Log) << replyData;
        const auto replyDoc = QJsonDocument::fromJson(replyData);
        const auto sci_sci_rp = replyDoc.object().value(QLatin1String("sci_sci_rp")).toObject();
        if (sci_sci_rp.contains(QLatin1String("ts")) && !sci_sci_rp.contains(QLatin1String("err_nr"))) {
            // TODO report success
            return;
        }

        // TODO report checking error
        const auto errNr = sci_sci_rp.value(QLatin1String("err_nr")).toInt();
        qCDebug(Log) << errNr;
    });
}
