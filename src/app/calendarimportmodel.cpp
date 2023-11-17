/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarimportmodel.h"

#include <KItinerary/Event>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>
#include <KItinerary/Visit>

#include <kcalendarcore_version.h>

#include <QDebug>
#include <QJsonArray>

CalendarImportModel::CalendarImportModel(QObject *parent)
    : QAbstractListModel(parent)
{
    qRegisterMetaType<QVector<QVariant>>();
}

CalendarImportModel::~CalendarImportModel() = default;

KCalendarCore::Calendar* CalendarImportModel::calendar() const
{
    return m_calendar;
}

void CalendarImportModel::setCalendar(KCalendarCore::Calendar *calendar)
{
    if (m_calendar == calendar) {
        return;
    }

    if (m_calendar) {
        disconnect(m_calendar, nullptr, this, nullptr);
    }

    m_calendar = calendar;
    Q_EMIT calendarChanged();
    if (!m_calendar) {
        return;
    }

#if KCALENDARCORE_VERSION >= QT_VERSION_CHECK(5, 96, 0)
    if (!m_calendar->isLoading()) {
        reload();
    } else {
        connect(m_calendar, &KCalendarCore::Calendar::isLoadingChanged, this, &CalendarImportModel::reload);
    }
#else
    reload();
#endif
}

QVector<QVariant> CalendarImportModel::selectedReservations() const
{
    QVector<QVariant> res;
    for (const auto &ev : m_events) {
        if (!ev.selected) {
            continue;
        }
        std::copy(ev.data.begin(), ev.data.end(), std::back_inserter(res));
    }
    return res;
}

int CalendarImportModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_events.size();
}

QVariant CalendarImportModel::data(const QModelIndex &index, int role) const
{
    using namespace KItinerary;
    if (!checkIndex(index)) {
        return {};
    }

    const auto &ev = m_events[index.row()];
    switch (role) {
        case TitleRole:
            return ev.event->summary();
        case SubtitleRole:
            return QLocale().toString(ev.event->dtStart());
        case IconNameRole:
            if (!ev.data.isEmpty()) {
                const auto res = ev.data.at(0);
                if (JsonLd::isA<FlightReservation>(res)) {
                    return QStringLiteral("qrc:///images/flight.svg");
                }
                if (JsonLd::isA<TrainReservation>(res)) {
                    return QStringLiteral("qrc:///images/train.svg");
                }
                if (JsonLd::isA<BusReservation>(res)) {
                    return QStringLiteral("qrc:///images/bus.svg");
                }
                if (JsonLd::isA<BoatReservation>(res)) {
                    return QStringLiteral("qrc:///images/ferry.svg");
                }
                if (JsonLd::isA<LodgingReservation>(res)) {
                    return QStringLiteral("go-home-symbolic");
                }
                if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
                    return QStringLiteral("qrc:///images/foodestablishment.svg");
                }
                if (JsonLd::isA<RentalCarReservation>(res)) {
                    return QStringLiteral("qrc:///images/car.svg");
                }
            }
            return QStringLiteral("meeting-attending");
        case ReservationsRole:
            return QVariant::fromValue(ev.data);
        case SelectedRole:
            return ev.selected;
    }

    return {};
}

bool CalendarImportModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto &ev = m_events[index.row()];
    switch (role) {
        case SelectedRole:
            ev.selected = value.toBool();
            Q_EMIT dataChanged(index, index);
            Q_EMIT hasSelectionChanged();
            return true;
    }
    return false;
}

QHash<int, QByteArray> CalendarImportModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(TitleRole, "title");
    n.insert(SubtitleRole, "subtitle");
    n.insert(IconNameRole, "iconName");
    n.insert(ReservationsRole, "reservations");
    n.insert(SelectedRole, "selected");
    return n;
}

bool CalendarImportModel::hasSelection() const
{
    return std::any_of(m_events.begin(), m_events.end(), [](const auto &ev) { return ev.selected; });
}

void CalendarImportModel::reload()
{
    beginResetModel();
    m_events.clear();

    KItinerary::ExtractorEngine extractorEngine;
    extractorEngine.setHints(KItinerary::ExtractorEngine::ExtractGenericIcalEvents);

    KItinerary::ExtractorValidator validator;
    validator.setAcceptedTypes<
        KItinerary::BoatReservation,
        KItinerary::BusReservation,
        KItinerary::Event,
        KItinerary::EventReservation,
        KItinerary::FlightReservation,
        KItinerary::FoodEstablishmentReservation,
        KItinerary::LodgingReservation,
        KItinerary::RentalCarReservation,
        KItinerary::TrainReservation,
        KItinerary::TouristAttractionVisit
    >();

    auto calEvents = m_calendar->events(today().addDays(-5), today().addDays(180));
    calEvents = m_calendar->sortEvents(std::move(calEvents), KCalendarCore::EventSortStartDate, KCalendarCore::SortDirectionAscending);
    for (const auto &ev : std::as_const(calEvents)) {
        extractorEngine.clear();
        extractorEngine.setContent(QVariant::fromValue(ev), u"internal/event");
        KItinerary::ExtractorPostprocessor postProc;
        postProc.process(KItinerary::JsonLdDocument::fromJson(extractorEngine.extract()));
        auto res = postProc.result();
        res.erase(std::remove_if(res.begin(), res.end(), [&validator](const auto &elem) { return !validator.isValidElement(elem); }), res.end());
        if (res.empty()) {
            continue;
        }

        const auto isReservation = !KItinerary::JsonLd::isA<KItinerary::Event>(res.at(0));
        m_events.push_back({ev, res, isReservation});
    }

    endResetModel();

    Q_EMIT hasSelectionChanged();
}

QDate CalendarImportModel::today() const
{
    if (Q_UNLIKELY(m_todayOverride.isValid())) {
        return m_todayOverride;
    }
    return QDate::currentDate();
}

#include "moc_calendarimportmodel.cpp"
