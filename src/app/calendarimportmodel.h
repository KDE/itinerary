/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CALENDARIMPORTMODEL_H
#define CALENDARIMPORTMODEL_H

#include <QAbstractListModel>

#include <KCalendarCore/Calendar>

/** List of possible events to import from a selected calendar. */
class CalendarImportModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(KCalendarCore::Calendar* calendar READ calendar WRITE setCalendar NOTIFY calendarChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged)

public:
    explicit CalendarImportModel(QObject *parent = nullptr);
    ~CalendarImportModel();

    enum Role {
        TitleRole = Qt::DisplayRole,
        SubtitleRole = Qt::UserRole,
        IconNameRole,
        ReservationsRole,
        SelectedRole,
    };

    KCalendarCore::Calendar *calendar() const;
    void setCalendar(KCalendarCore::Calendar *calendar);

    Q_INVOKABLE QList<QVariant> selectedReservations() const;

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    bool hasSelection() const;

Q_SIGNALS:
    void calendarChanged();
    void hasSelectionChanged();

private:
    void reload();
    QDate today() const;

    KCalendarCore::Calendar *m_calendar = nullptr;

    struct Event {
        KCalendarCore::Event::Ptr event;
        QList<QVariant> data;
        bool selected;
    };
    std::vector<Event> m_events;

    friend class CalendarImportModelTest;
    QDate m_todayOverride;
};

#endif // CALENDARIMPORTMODEL_H
